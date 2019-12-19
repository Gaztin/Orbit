/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will
 * the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the
 *    original software. If you use this software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as
 *    being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "Window.h"

#include "Orbit/Core/Input/Input.h"
#include "Orbit/Core/Input/Key.h"
#include "Orbit/Core/Platform/Android/AndroidApp.h"
#include "Orbit/Core/Platform/Android/AndroidNativeAppGlue.h"
#include "Orbit/Core/Platform/iOS/UIWindow.h"
#include "Orbit/Core/Platform/macOS/WindowDelegate.h"
#include "Orbit/Core/Utility/Utility.h"

#if defined( ORB_OS_WINDOWS )
#  include <windowsx.h>
#elif defined( ORB_OS_MACOS )
#  include <AppKit/AppKit.h>
#elif defined( ORB_OS_ANDROID )
#  include <android/sensor.h>
#endif

ORB_NAMESPACE_BEGIN

#if defined( ORB_OS_WINDOWS )
static ATOM RegisterWindowClass( LPCSTR name );
#elif defined( ORB_OS_LINUX )
static void HandleXEvent( Window* w, const XEvent& xevent );
#elif defined( ORB_OS_ANDROID )
static void AppCMD ( AndroidApp* state, AndroidAppCommand cmd );
static int  OnInput( AndroidApp* state, AInputEvent* e );
#endif

Window::Window( [[ maybe_unused ]] uint32_t width, [[ maybe_unused ]] uint32_t height )
	: m_details { }
	, m_open    { true }
{

#if defined( ORB_OS_WINDOWS )

	constexpr LPCSTR class_name   = "OrbitWindow";
	static ATOM      window_class = RegisterWindowClass( class_name );

	/* Create window */
	m_details.hwnd = CreateWindowExA( WS_EX_OVERLAPPEDWINDOW, class_name, NULL, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandleA( NULL ), NULL );

	/* Set user data */
	SetWindowLongPtrA( m_details.hwnd, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( this ) );

#elif defined( ORB_OS_LINUX )

	/* Open display */
	m_details.display = XOpenDisplay( nullptr );

	/* Create window */
	const int               screen      = DefaultScreen( m_details.display );
	XID                     root_window = XRootWindow( m_details.display, screen );
	int                     depth       = DefaultDepth( m_details.display, screen );
	Visual*                 visual      = DefaultVisual( m_details.display, screen );
	constexpr unsigned long value_mask  = ( CWBackPixel | CWEventMask );
	XSetWindowAttributes    attribs     = { };
	attribs.event_mask                  = ( FocusChangeMask | ResizeRedirectMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask );
	m_details.window                    = XCreateWindow( m_details.display, root_window, 0, 0, width, height, 0, depth, InputOutput, visual, value_mask, &attribs );

	/* Allow us to capture the window close event */
	Atom close_atom = XInternAtom( m_details.display, "WM_DELETE_WINDOW", True );
	XSetWMProtocols( m_details.display, m_details.window, &close_atom, 1 );
	
	/* Send initial resize event */
	XEvent xevent;
	xevent.xresizerequest.type       = ResizeRequest;
	xevent.xresizerequest.display    = m_details.display;
	xevent.xresizerequest.window     = m_details.window;
	xevent.xresizerequest.width      = width;
	xevent.xresizerequest.height     = height;
	xevent.xresizerequest.send_event = True;
	xevent.xresizerequest.serial     = 0;
	XSendEvent( m_details.display, m_details.window, False, 0, &xevent );

#elif defined( ORB_OS_MACOS )

	NSRect             frame      = NSMakeRect( 0.0f, 0.0f, width, height );
	NSWindowStyleMask  style_mask = ( NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable );
	NSBackingStoreType backing    = ( NSBackingStoreBuffered );

	/* Create window */
	m_details.window = [ NSWindow alloc ];
	[ m_details.window initWithContentRect:frame styleMask:style_mask backing:backing defer:NO ];

	/* Create window delegate */
	m_details.delegate = [ OrbitWindowDelegate alloc ];
	[ m_details.window setDelegate:m_details.delegate ];

	/* Send initial resize event */
	WindowResized e;
	e.width  = width;
	e.height = height;
	QueueEvent( e );

#elif defined( ORB_OS_ANDROID )

	AndroidOnly::app->on_input_event = OnInput;

	m_details.sensor_manager       = ASensorManager_getInstance();
	m_details.accelerometer_sensor = ASensorManager_getDefaultSensor( m_details.sensor_manager, ASENSOR_TYPE_ACCELEROMETER );
	m_details.sensor_event_queue   = ASensorManager_createEventQueue( m_details.sensor_manager, AndroidOnly::app->looper, static_cast< int >( AndroidLooperID::User ), nullptr, nullptr );

	AndroidOnly::app->user_data  = this;
	AndroidOnly::app->on_app_cmd = AppCMD;

	/* Update until native window is initialized */
	while( !AndroidOnly::app->window )
	{
		AndroidPollSource* source;

		if( ALooper_pollAll( 0, nullptr, nullptr, reinterpret_cast< void** >( &source ) ) >= 0 && source )
		{
			source->process( AndroidOnly::app, source );
		}
	}

#elif defined( ORB_OS_IOS )

	/* Initialize window */
	m_details.ui_window = [ OrbitUIWindow alloc ];
	[ m_details.ui_window initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
	m_details.ui_window.backgroundColor = [ UIColor whiteColor ];
	[ m_details.ui_window makeKeyAndVisible ];

	/* Create view controller */
	UIViewController* vc = [ UIViewController alloc ];
	[ vc initWithNibName:nil bundle:nil ];
	m_details.ui_window.rootViewController = vc;

#endif

}

Window::~Window( void )
{

#if defined( ORB_OS_WINDOWS )

	DestroyWindow( m_details.hwnd );

#elif defined( ORB_OS_LINUX )

	XDestroyWindow( m_details.display, m_details.window );
	XCloseDisplay( m_details.display );

#elif defined( ORB_OS_MACOS )

	[ m_details.window close ];
	[ m_details.delegate dealloc ];
	[ m_details.window dealloc ];

#elif defined( ORB_OS_ANDROID )

	ASensorManager_destroyEventQueue( m_details.sensor_manager, m_details.sensor_event_queue );
	AndroidOnly::app->user_data  = nullptr;
	AndroidOnly::app->on_app_cmd = nullptr;

#elif defined( ORB_OS_IOS )

	[ m_details.ui_window dealloc ];

#endif

}

void Window::PollEvents( void )
{
	Input::ResetStates();

#if defined( ORB_OS_WINDOWS )

	MSG msg;

	while( PeekMessageA( &msg, m_details.hwnd, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}

#elif defined( ORB_OS_LINUX )

	while( XPending( m_details.display ) )
	{
		XEvent xevent;
		XNextEvent( m_details.display, &xevent );

		HandleXEvent( this, xevent );
	}

#elif defined( ORB_OS_MACOS )

	NSEvent* event;

	while( ( event = [ m_details.window nextEventMatchingMask:NSEventMaskAny untilDate:nullptr inMode:NSDefaultRunLoopMode dequeue:YES ] ) != nullptr )
	{
		[ m_details.window sendEvent:event ];
	}

#elif defined( ORB_OS_ANDROID )

	AndroidPollSource* source;

	if( ALooper_pollAll( 0, nullptr, nullptr, reinterpret_cast< void** >( &source ) ) >= 0 && source )
	{
		source->process( AndroidOnly::app, source );
	}

#endif

	/* Send events */
	SendEvents();
}

void Window::SetTitle( std::string_view title )
{

#if defined( ORB_OS_WINDOWS )

	SetWindowTextA( m_details.hwnd, title.data() );

#elif defined( ORB_OS_LINUX )

	XStoreName( m_details.display, m_details.window, title.data() );

#elif defined( ORB_OS_MACOS )

	NSString* title_objc = [ NSString stringWithUTF8String:title.data() ];
	[ m_details.window setTitle:title_objc ];
	[ title_objc release ];

#elif defined( ORB_OS_ANDROID )

	// #TODO: Activity.setTitle
	( void )title;

#endif

}

void Window::Move( [[ maybe_unused ]] int32_t x, [[ maybe_unused ]] int32_t y )
{

#if defined( ORB_OS_WINDOWS )

	RECT rect;

	if( GetWindowRect( m_details.hwnd, &rect ) )
	{
		const int width  = ( rect.right - rect.left );
		const int height = ( rect.bottom - rect.top );

		MoveWindow( m_details.hwnd, x, y, width, height, FALSE );
	}

#elif defined( ORB_OS_LINUX )

	XMoveWindow( m_details.display, m_details.window, x, y );

#elif defined( ORB_OS_MACOS )

	NSRect frame   = [ m_details.window frame ];
	frame.origin.x = x;
	frame.origin.y = y;

	[ m_details.window setFrame:frame display:YES ];

#endif

}

void Window::Resize( [[ maybe_unused ]] uint32_t width, [[ maybe_unused ]] uint32_t height )
{

#if defined( ORB_OS_WINDOWS )

	RECT rect;
	
	if( GetWindowRect( m_details.hwnd, &rect ) )
	{
		MoveWindow( m_details.hwnd, rect.left, rect.top, width, height, TRUE );
	}

#elif defined( ORB_OS_LINUX )

	XResizeWindow( m_details.display, m_details.window, width, height );

#elif defined( ORB_OS_MACOS )

	NSRect frame      = [ m_details.window frame ];
	frame.size.width  = width;
	frame.size.height = height;

	[ m_details.window setFrame:frame display:YES ];

#endif

}

void Window::Show( void )
{

#if defined( ORB_OS_WINDOWS )

	ShowWindow( m_details.hwnd, SW_SHOW );

#elif defined( ORB_OS_LINUX )

	XMapWindow( m_details.display, m_details.window );

#elif defined( ORB_OS_MACOS )

	[ m_details.window setIsVisible:YES ];

#elif defined( ORB_OS_ANDROID )

	// #TODO: Activity.setVisible

#elif defined( ORB_OS_IOS )

	[ ( OrbitUIWindow* )m_details.ui_window setHidden:NO ];

#endif

}

void Window::Hide( void )
{

#if defined( ORB_OS_WINDOWS )

	ShowWindow( m_details.hwnd, SW_HIDE );

#elif defined( ORB_OS_LINUX )

	XUnmapWindow( m_details.display, m_details.window );

#elif defined( ORB_OS_MACOS )

	[ m_details.window setIsVisible:NO ];

#elif defined( ORB_OS_ANDROID )

	// #TODO: Activity.setVisible

#elif defined( ORB_OS_IOS )

	[ m_details.ui_window setHidden:YES ];

#endif

}

void Window::Close( void )
{
	m_open = false;
}

#if defined( ORB_OS_WINDOWS )

static LRESULT WINAPI WindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	Window* w = reinterpret_cast< Window* >( GetWindowLongPtrA( hwnd, GWLP_USERDATA ) );

	switch( msg )
	{
		case WM_MOVE:
		{
			WindowMoved e;
			e.x = LOWORD( lparam );
			e.y = HIWORD( lparam );

			w->QueueEvent( e );

			break;
		}

		case WM_SIZE:
		{
			WindowResized e;
			e.width  = LOWORD( lparam );
			e.height = HIWORD( lparam );

			w->QueueEvent( e );

			break;
		}

		case WM_ACTIVATE:
		{
			WORD minimized_state = HIWORD( wparam );
			WORD activated       = LOWORD( wparam );

			if( minimized_state != 0 )
			{
				WindowStateChanged e;
				e.state = ( activated == WA_INACTIVE ) ? WindowState::Suspend : WindowState::Restore;

				w->QueueEvent( e );
			}

			break;
		}

		case WM_SETFOCUS:
		{
			WindowStateChanged e;
			e.state = WindowState::Focus;

			w->QueueEvent( e );

			break;
		}

		case WM_KILLFOCUS:
		{
			WindowStateChanged e;
			e.state = WindowState::Defocus;

			w->QueueEvent( e );

			break;
		}

		case WM_CLOSE:
		{
			WindowStateChanged e;
			e.state = WindowState::Close;

			w->QueueEvent( e );
			w->Close();

			break;
		}

		case WM_KEYDOWN:
		{
			if( ( HIWORD( lparam ) & KF_REPEAT ) == 0 )
			{
				const Key key = ConvertSystemKey( static_cast< uint32_t >( wparam ) );

				Input::SetKeyPressed( key );
			}

			break;
		}

		case WM_KEYUP:
		{
			const Key key = ConvertSystemKey( static_cast< uint32_t >( wparam ) );
			Input::SetKeyReleased( key );

			break;
		}

		case WM_LBUTTONDOWN:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerPressed( 0, pos );

			break;
		}

		case WM_LBUTTONUP:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerReleased( 0, pos );

			break;
		}

		case WM_RBUTTONDOWN:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerPressed( 1, pos );

			break;
		}

		case WM_RBUTTONUP:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerReleased( 1, pos );

			break;
		}

		case WM_MOUSEMOVE:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );

			if( wparam & MK_LBUTTON )
			{
				Input::SetPointerPos( 0, pos );
			}

			if( wparam & MK_RBUTTON )
			{
				Input::SetPointerPos( 1, pos );
			}

			break;
		}
	}

	return DefWindowProcA( hwnd, msg, wparam, lparam );
}

ATOM RegisterWindowClass( LPCSTR name )
{
	WNDCLASSEXA classDesc { };
	classDesc.cbSize        = sizeof( WNDCLASSEXA );
	classDesc.style         = CS_VREDRAW | CS_HREDRAW;
	classDesc.lpfnWndProc   = WindowProc;
	classDesc.hInstance     = GetModuleHandleA( nullptr );
	classDesc.hbrBackground = reinterpret_cast< HBRUSH >( COLOR_WINDOW );
	classDesc.lpszClassName = name;

	/* Extract and copy icon from application. */
	char path[ MAX_PATH + 1 ];
	GetModuleFileNameA( nullptr, path, sizeof( path ) );
	classDesc.hIcon = ExtractIconA( classDesc.hInstance, path, 0 );

	return RegisterClassExA( &classDesc );
}

#elif defined( ORB_OS_LINUX )

void HandleXEvent( Window* w, const XEvent& xevent )
{
	switch( xevent.type )
	{
		default: break;

		case FocusIn:
		{
			if( xevent.xfocus.mode == NotifyNormal )
			{
				WindowStateChanged e;
				e.state = WindowState::Focus;

				w->QueueEvent( e );
			}

			break;
		}

		case FocusOut:
		{
			if( xevent.xfocus.mode == NotifyNormal )
			{
				WindowStateChanged e;
				e.state = WindowState::Defocus;

				w->QueueEvent( e );
			}

			break;
		}

		case ResizeRequest:
		{
			WindowResized e;
			e.width  = xevent.xresizerequest.width;
			e.height = xevent.xresizerequest.height;

			w->QueueEvent( e );

			break;
		}

		case ConfigureNotify:
		{
			WindowMoved e;
			e.x = xevent.xconfigure.x;
			e.y = xevent.xconfigure.y;

			w->QueueEvent( e );

			break;
		}

		case ClientMessage:
		{
			w->Close();

			break;
		}
		
		case KeyPress:
		{
			Key key = ConvertSystemKey( xevent.xkey.keycode );
			
			Input::SetKeyPressed( key );
			
			break;
		}
		
		case KeyRelease:
		{
			Key key = ConvertSystemKey( xevent.xkey.keycode );
			
			Input::SetKeyReleased( key );
			
			break;
		}
	}
}

#elif defined( ORB_OS_ANDROID )

void AppCMD( AndroidApp* state, AndroidAppCommand cmd )
{
	Window* w = static_cast< Window* >( state->user_data );

	switch( cmd )
	{
		default: break;

		case AndroidAppCommand::InitWindow:
		{
			WindowResized e1;
			e1.width  = static_cast< uint32_t >( ANativeWindow_getWidth( state->window ) );
			e1.height = static_cast< uint32_t >( ANativeWindow_getHeight( state->window ) );

			WindowStateChanged e2;
			e2.state = WindowState::Restore;

			w->QueueEvent( e1 );
			w->QueueEvent( e2 );

			break;
		}

		case AndroidAppCommand::TermWindow:
		{
			WindowStateChanged e;
			e.state = WindowState::Suspend;

			w->QueueEvent( e );

			break;
		}

		case AndroidAppCommand::GainedFocus:
		{
			auto& data = w->GetPrivateDetails();
			ASensorEventQueue_enableSensor( data.sensor_event_queue, data.accelerometer_sensor );
			ASensorEventQueue_setEventRate( data.sensor_event_queue, data.accelerometer_sensor, ( 1000 * 1000 / 60 ) );

			WindowStateChanged e;
			e.state = WindowState::Focus;

			w->QueueEvent( e );

			break;
		}

		case AndroidAppCommand::LostFocus:
		{
			auto& data = w->GetPrivateDetails();
			ASensorEventQueue_disableSensor( data.sensor_event_queue, data.accelerometer_sensor );

			WindowStateChanged e;
			e.state = WindowState::Defocus;

			w->QueueEvent( e );

			break;
		}

		case AndroidAppCommand::Destroy:
		{
			w->Close();
			
			break;
		}
	}
};

static int OnInput( AndroidApp* /*state*/, AInputEvent* e )
{
	switch( AInputEvent_getType( e ) )
	{
		default: break;
	}

	return 0;
};

#endif

ORB_NAMESPACE_END
