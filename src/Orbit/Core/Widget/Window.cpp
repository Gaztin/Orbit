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

#include "Orbit/Core/Platform/Android/AndroidApp.h"
#include "Orbit/Core/Utility/Utility.h"

#if defined( ORB_OS_ANDROID )
#  include <android/sensor.h>
#  include <android_native_app_glue.h>
#endif

#if defined( ORB_OS_MACOS )
#  include <AppKit/AppKit.h>
@interface OrbitWindowDelegate : NSObject< NSWindowDelegate >
@end
#endif

#if defined( ORB_OS_IOS )
#  include <UIKit/UIKit.h>
@interface OrbitUIWindow : UIWindow
@end
#endif

ORB_NAMESPACE_BEGIN

#if defined( ORB_OS_WINDOWS )
static ATOM RegisterWindowClass( LPCSTR name );
#elif defined( ORB_OS_LINUX )
static void HandleXEvent( Window* w, const XEvent& xevent );
#elif defined( ORB_OS_ANDROID )
static void AppCMD( android_app* state, int cmd );
static int OnInput( android_app* state, AInputEvent* e );
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
	attribs.event_mask                  = ( FocusChangeMask | ResizeRedirectMask | StructureNotifyMask );
	m_details.window                    = XCreateWindow( m_details.display, root_window, 0, 0, width, height, 0, depth, InputOutput, visual, value_mask, &attribs );

	/* Allow us to capture the window close event */
	Atom close_atom = XInternAtom( m_details.display, "WM_DELETE_WINDOW", True );
	XSetWMProtocols( m_details.display, m_details.window, &close_atom, 1 );

#elif defined( ORB_OS_MACOS )

	NSRect             frame      = NSMakeRect( 0.0f, 0.0f, width, height );
	NSWindowStyleMask  style_mask = ( NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable );
	NSBackingStoreType backing    = ( NSBackingStoreBuffered );

	/* Create window */
	m_details.ns_window = [ NSWindow alloc ];
	[ ( NSWindow* )m_details.ns_window initWithContentRect:frame styleMask:style_mask backing:backing defer:NO ];

	/* Create window delegate */
	m_details.delegate = [ OrbitWindowDelegate alloc ];
	[ ( NSWindow* )m_details.ns_window setDelegate:( OrbitWindowDelegate* )m_details.delegate ];

#elif defined( ORB_OS_ANDROID )

	AndroidOnly::app->onInputEvent = OnInput;

	m_details.sensor_manager       = ASensorManager_getInstance();
	m_details.accelerometer_sensor = ASensorManager_getDefaultSensor( m_details.sensor_manager, ASENSOR_TYPE_ACCELEROMETER );
	m_details.sensor_event_queue   = ASensorManager_createEventQueue( m_details.sensor_manager, AndroidOnly::app->looper, LOOPER_ID_USER, nullptr, nullptr );

	AndroidOnly::app->userData = this;
	AndroidOnly::app->onAppCmd = AppCMD;

	/* Update until native window is initialized */
	while( !AndroidOnly::app->window )
	{
		android_poll_source* source;

		if( ALooper_pollAll( 0, nullptr, nullptr, reinterpret_cast< void** >( &source ) ) >= 0 && source )
		{
			source->process( AndroidOnly::app, source );
		}
	}

#elif defined( ORB_OS_IOS )

	/* Initialize window */
	m_details.ui_window = [ OrbitUIWindow alloc ];
	[ ( OrbitUIWindow* )m_details.ui_window initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
	( ( OrbitUIWindow* )m_details.ui_window ).backgroundColor = [ UIColor whiteColor ];
	[ ( OrbitUIWindow* )m_details.ui_window makeKeyAndVisible ];

	/* Create view controller */
	UIViewController* vc = [ UIViewController alloc ];
	[ vc initWithNibName:nil bundle:nil ];
	( ( OrbitUIWindow* )m_details.ui_window ).rootViewController = vc;

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

	[ ( NSWindow* )m_details.ns_window close ];
	[ ( OrbitWindowDelegate* )m_details.delegate dealloc ];
	[ ( NSWindow* )m_details.nsWindow dealloc ];

#elif defined( ORB_OS_ANDROID )

	ASensorManager_destroyEventQueue( m_details.sensor_manager, m_details.sensor_event_queue );
	AndroidOnly::app->userData = nullptr;
	AndroidOnly::app->onAppCmd = nullptr;

#elif defined( ORB_OS_IOS )

	[ ( OrbitUIWindow* )m_details.ui_window dealloc ];

#endif

}

void Window::PollEvents( void )
{

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

	NSEvent* ns_event;

	while( ( ns_event = [ ( const NSWindow* )m_details.ns_window nextEventMatchingMask : NSEventMaskAny untilDate : nullptr inMode : NSDefaultRunLoopMode dequeue : YES ] ) != nullptr )
	{
		[ ( const NSWindow* )m_details.ns_window sendEvent : ns_event ];
	}

#elif defined( ORB_OS_ANDROID )

	android_poll_source* source;

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

	NSString* ns_title = [ NSString stringWithUTF8String:title.data() ];
	[ ( const NSWindow* )m_details.ns_window setTitle:ns_title ];
	[ nsTitle release ];

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

	NSRect frame   = [ ( const NSWindow* )m_details.ns_window frame ];
	frame.origin.x = x;
	frame.origin.y = y;

	[ ( const NSWindow* )m_details.ns_window setFrame:frame display:YES ];

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

	NSRect frame      = [ ( const NSWindow* )m_details.ns_window frame ];
	frame.size.width  = width;
	frame.size.height = height;

	[ ( const NSWindow* )m_details.ns_window setFrame:frame display:YES ];

#endif

}

void Window::Show( void )
{

#if defined( ORB_OS_WINDOWS )

	ShowWindow( m_details.hwnd, SW_SHOW );

#elif defined( ORB_OS_LINUX )

	XMapWindow( m_details.display, m_details.window );

#elif defined( ORB_OS_MACOS )

	[ ( const NSWindow* )m_details.ns_window setIsVisible:YES ];

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

	[ ( const NSWindow* )m_details.ns_window setIsVisible:NO ];

#elif defined( ORB_OS_ANDROID )

	// #TODO: Activity.setVisible

#elif defined( ORB_OS_IOS )

	[ ( OrbitUIWindow* )m_details.ui_window setHidden:YES ];

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
	}
}

#elif defined( ORB_OS_ANDROID )

void AppCMD( android_app* state, int cmd )
{
	Window* w = static_cast< Window* >( state->userData );

	switch( cmd )
	{
		default: break;

		case APP_CMD_INIT_WINDOW:
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

		case APP_CMD_TERM_WINDOW:
		{
			WindowStateChanged e;
			e.state = WindowState::Suspend;

			w->QueueEvent( e );

			break;
		}

		case APP_CMD_GAINED_FOCUS:
		{
			auto& data = w->GetPrivateDetails();
			ASensorEventQueue_enableSensor( data.sensor_event_queue, data.accelerometer_sensor );
			ASensorEventQueue_setEventRate( data.sensor_event_queue, data.accelerometer_sensor, ( 1000 * 1000 / 60 ) );

			WindowStateChanged e;
			e.state = WindowState::Focus;

			w->QueueEvent( e );

			break;
		}

		case APP_CMD_LOST_FOCUS:
		{
			auto& data = w->GetPrivateDetails();
			ASensorEventQueue_disableSensor( data.sensor_event_queue, data.accelerometer_sensor );

			WindowStateChanged e;
			e.state = WindowState::Defocus;

			w->QueueEvent( e );

			break;
		}

		case APP_CMD_DESTROY:
		{
			w->Close();
			
			break;
		}
	}
};

static int OnInput( android_app* state, AInputEvent* e )
{
	switch( AInputEvent_getType( e ) )
	{
		default: break;
	}

	return 0;
};

#endif

ORB_NAMESPACE_END

#if defined( ORB_OS_MACOS )

@implementation OrbitWindowDelegate

-( void )windowWillClose:( NSNotification* ) __unused notification
{
	Window::GetInstance().Close();
}

-( void )windowDidMove:( NSNotification* ) __unused notification
{
	Window&        window  = Window::GetInstance();
	WindowDetails& details = window.GetPrivateDetails();
	const CGPoint  point   = ( ( const NSWindow* )details.ns_window ).frame.origin;

	ORB_NAMESPACE WindowMoved e;
	e.x = point.x;
	e.y = point.y;

	window.QueueEvent( e );
}

-( NSSize )windowWillResize:( NSWindow* ) __unused sender toSize:( NSSize ) frameSize
{
	ORB_NAMESPACE WindowResized e;
	e.width  = frameSize.width;
	e.height = frameSize.height;

	Window::GetInstance().QueueEvent( e );

	return frameSize;
}

-( void )windowDidMiniaturize:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowStateChanged e;
	e.state = ORB_NAMESPACE WindowState::Suspend;

	Window::GetInstance().QueueEvent( e );
}

-( void )windowDidDeminiaturize:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowStateChanged e;
	e.state = ORB_NAMESPACE WindowState::Restore;

	Window::GetInstance().QueueEvent( e );
}

-( void )windowDidBecomeMain:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowStateChanged e;
	e.state = ORB_NAMESPACE WindowState::Focus;

	Window::GetInstance().QueueEvent( e );
}

-( void )windowDidResignMain:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowStateChanged e;
	e.state = ORB_NAMESPACE WindowState::Defocus;

	Window::GetInstance().QueueEvent( e );
}

@end

#elif defined( ORB_OS_IOS )

@implementation OrbitUIWindow

-( void )layoutSubviews
{
	[ super layoutSubviews ];

	ORB_NAMESPACE WindowResized e;
	e.width  = self.bounds.size.width;
	e.height = self.bounds.size.height;

	Window::GetInstance().QueueEvent( e );
}

@end

#endif
