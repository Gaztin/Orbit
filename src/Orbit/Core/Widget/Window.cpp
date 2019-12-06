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

Window::Window( uint32_t width, uint32_t height )
	: m_data { }
	, m_open { true }
{

#if defined( ORB_OS_WINDOWS )

	constexpr LPCSTR class_name   = "OrbitWindow";
	static ATOM      window_class = RegisterWindowClass( class_name );

	/* Create window */
	m_data.hwnd = CreateWindowExA( WS_EX_OVERLAPPEDWINDOW, class_name, NULL, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandleA( NULL ), NULL );

	/* Set user data */
	SetWindowLongPtrA( m_data.hwnd, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( this ) );

#elif defined( ORB_OS_LINUX )

	/* Open display */
	m_data.display = XOpenDisplay( nullptr );

	/* Create window */
	const int               screen      = DefaultScreen( m_data.display );
	Window                  root_window = XRootWindow( m_data.display, screen );
	int                     depth       = DefaultDepth( m_data.display, screen );
	Visual*                 visual      = DefaultVisual( m_data.display, screen );
	constexpr unsigned long value_mask  = ( CWBackPixel | CWEventMask );
	XSetWindowAttributes    attribs     = { };
	attribs.event_mask                  = ( FocusChangeMask | ResizeRedirectMask | StructureNotifyMask );
	m_data.window                       = XCreateWindow( m_data.display, root_window, 0, 0, width, height, 0, depth, InputOutput, visual, value_mask, &attribs );

	/* Allow us to capture the window close event */
	Atom close_atom = XInternAtom( m_data.display, "WM_DELETE_WINDOW", True );
	XSetWMProtocols( m_data.display, m_data.window, &close_atom, 1 );

#elif defined( ORB_OS_MACOS )

	NSRect             frame      = NSMakeRect( 0.0f, 0.0f, width, height );
	NSWindowStyleMask  style_mask = ( NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable );
	NSBackingStoreType backing    = ( NSBackingStoreBuffered );

	/* Create window */
	m_data.ns_window = [ NSWindow alloc ];
	[ ( NSWindow* )m_data.ns_window initWithContentRect:frame styleMask:style_mask backing:backing defer:NO ];

	/* Create window delegate */
	m_data.delegate = [ OrbitWindowDelegate alloc ];
	[ ( NSWindow* )m_data.ns_window setDelegate:( OrbitWindowDelegate* )m_data.delegate ];

#elif defined( ORB_OS_ANDROID )

	AndroidOnly::app->onInputEvent = OnInput;

	m_data.sensor_manager       = ASensorManager_getInstance();
	m_data.accelerometer_sensor = ASensorManager_getDefaultSensor( m_data.sensor_manager, ASENSOR_TYPE_ACCELEROMETER );
	m_data.sensor_event_queue   = ASensorManager_createEventQueue( m_data.sensor_manager, AndroidOnly::app->looper, LOOPER_ID_USER, nullptr, nullptr );

	/* Update until native window is initialized. */
	{
		bool initialized = false;
		AndroidOnly::app->userData = &initialized;
		AndroidOnly::app->onAppCmd = []( android_app* state, int cmd )
		{
			if( cmd == APP_CMD_INIT_WINDOW )
			{
				auto initialized = static_cast< bool* >( state->userData );
				*initialized = true;
			}
		};

		while( !initialized )
		{
			PollEvents();
		}
	}

	AndroidOnly::app->userData = this;
	AndroidOnly::app->onAppCmd = AppCMD;

#elif defined( ORB_OS_IOS )

	/* Initialize window */
	m_data.ui_window = [ OrbitUIWindow alloc ];
	[ ( OrbitUIWindow* )m_data.ui_window initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
	( ( OrbitUIWindow* )m_data.ui_window ).backgroundColor = [ UIColor whiteColor ];
	[ ( OrbitUIWindow* )m_data.ui_window makeKeyAndVisible ];

	/* Create view controller */
	UIViewController* vc = [ UIViewController alloc ];
	[ vc initWithNibName:nil bundle:nil ];
	( ( OrbitUIWindow* )m_data.ui_window ).rootViewController = vc;

#endif

}

Window::~Window( void )
{

#if defined( ORB_OS_WINDOWS )

	DestroyWindow( m_data.hwnd );

#elif defined( ORB_OS_LINUX )

	XDestroyWindow( m_data.display, m_data.window );
	XCloseDisplay( m_data.display );

#elif defined( ORB_OS_MACOS )

	[ ( NSWindow* )m_data.ns_window close ];
	[ ( OrbitWindowDelegate* )m_data.delegate dealloc ];
	[ ( NSWindow* )m_data.nsWindow dealloc ];

#elif defined( ORB_OS_ANDROID )

	ASensorManager_destroyEventQueue( m_data.sensor_manager, m_data.sensor_event_queue );
	AndroidOnly::app->userData = nullptr;
	AndroidOnly::app->onAppCmd = nullptr;

#elif defined( ORB_OS_IOS )

	[ ( OrbitUIWindow* )m_data.ui_window dealloc ];

#endif

}

void Window::PollEvents( void )
{

#if defined( ORB_OS_WINDOWS )

	MSG msg;

	while( PeekMessageA( &msg, m_data.hwnd, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}

#elif defined( ORB_OS_LINUX )

	while( XPending( impl->display ) )
	{
		XEvent xevent;
		XNextEvent( impl->display, &xevent );

		HandleXEvent( this, xevent );
	}

#elif defined( ORB_OS_MACOS )

	NSEvent* ns_event;

	while( ( ns_event = [ ( const NSWindow* )m_data.ns_window nextEventMatchingMask : NSEventMaskAny untilDate : nullptr inMode : NSDefaultRunLoopMode dequeue : YES ] ) != nullptr )
	{
		[ ( const NSWindow* )m_data.ns_window sendEvent : ns_event ];
	}

#elif defined( ORB_OS_ANDROID )

	int                  events;
	android_poll_source* source;

	if( ALooper_pollAll( 0, nullptr, &events, reinterpret_cast< void** >( &source ) ) >= 0 )
	{
		if( source )
		{
			source->process( AndroidOnly::app, source );
		}
	}

#endif

	/* Send events */
	SendEvents();
}

void Window::SetTitle( std::string_view title )
{

#if defined( ORB_OS_WINDOWS )

	SetWindowTextA( m_data.hwnd, title.data() );

#elif defined( ORB_OS_LINUX )

	XStore( m_data.display, m_data.window, title.data() );

#elif defined( ORB_OS_MACOS )

	NSString* ns_title = [ NSString stringWithUTF8String:title.data() ];
	[ ( const NSWindow* )m_data.ns_window setTitle:ns_title ];
	[ nsTitle release ];

#elif defined( ORB_OS_ANDROID )

	// #TODO: Activity.setTitle

#endif

}

void Window::Move( int32_t x, int32_t y )
{

#if defined( ORB_OS_WINDOWS )

	RECT rect;

	if( GetWindowRect( m_data.hwnd, &rect ) )
	{
		const int width  = ( rect.right - rect.left );
		const int height = ( rect.bottom - rect.top );

		MoveWindow( m_data.hwnd, x, y, width, height, FALSE );
	}

#elif defined( ORB_OS_LINUX )

	XMoveWindow( m_data.display, m_data.window, x, y );

#elif defined( ORB_OS_MACOS )

	NSRect frame   = [ ( const NSWindow* )m_data.ns_window frame ];
	frame.origin.x = x;
	frame.origin.y = y;

	[ ( const NSWindow* )m_data.ns_window setFrame:frame display:YES ];

#endif

}

void Window::Resize( uint32_t width, uint32_t height )
{

#if defined( ORB_OS_WINDOWS )

	RECT rect;
	
	if( GetWindowRect( m_data.hwnd, &rect ) )
	{
		MoveWindow( m_data.hwnd, rect.left, rect.top, width, height, TRUE );
	}

#elif defined( ORB_OS_LINUX )

	XResizeWindow( m_data.display, m_data.window, width, height );

#elif defined( ORB_OS_MACOS )

	NSRect frame      = [ ( const NSWindow* )m_data.ns_window frame ];
	frame.size.width  = width;
	frame.size.height = height;

	[ ( const NSWindow* )m_data.ns_window setFrame:frame display:YES ];

#endif

}

void Window::Show( void )
{

#if defined( ORB_OS_WINDOWS )

	ShowWindow( m_data.hwnd, SW_SHOW );

#elif defined( ORB_OS_LINUX )

	XMapWindow( m_data.display, m_data.window );

#elif defined( ORB_OS_MACOS )

	[ ( const NSWindow* )m_data.ns_window setIsVisible:YES ];

#elif defined( ORB_OS_ANDROID )

	// #TODO: Activity.setVisible

#elif defined( ORB_OS_IOS )

	[ ( OrbitUIWindow* )m_data.ui_window setHidden:NO ];

#endif

}

void Window::Hide( void )
{

#if defined( ORB_OS_WINDOWS )

	ShowWindow( m_data.hwnd, SW_HIDE );

#elif defined( ORB_OS_LINUX )

	XUnmapWindow( m_data.display, m_data.window );

#elif defined( ORB_OS_MACOS )

	[ ( const NSWindow* )m_data.ns_window setIsVisible:NO ];

#elif defined( ORB_OS_ANDROID )

	// #TODO: Activity.setVisible

#elif defined( ORB_OS_IOS )

	[ ( OrbitUIWindow* )m_data.ui_window setHidden:YES ];

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
			{
				WindowResized e;
				e.width  = static_cast< uint32_t >( ANativeWindow_getWidth( state->window ) );
				e.height = static_cast< uint32_t >( ANativeWindow_getHeight( state->window ) );

				w->QueueEvent( resize_event );
			}

			{
				WindowStateChanged e;
				e.state = WindowState::Restore;

				w->QueueEvent( e );
			}

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
			WindowData& impl = w->GetPrivateData();
			ASensorEventQueue_enableSensor( impl.sensor_event_queue, impl.accelerometer_sensor );
			ASensorEventQueue_setEventRate( impl.sensor_event_queue, impl.accelerometer_sensor, ( 1000 * 1000 / 60 ) );

			WindowStateChanged e;
			e.state = WindowState::Focus;

			w->QueueEvent( e );

			break;
		}

		case APP_CMD_LOST_FOCUS:
		{
			WindowData& impl = w->GetPrivateData();
			ASensorEventQueue_disableSensor( impl.sensor_event_queue, impl.accelerometer_sensor );

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
	WindowData&   impl  = _window_ptr->GetPrivateData();
	const CGPoint point = ( ( const NSWindow* )impl.ns_window ).frame.origin;

	ORB_NAMESPACE WindowMoved e;
	e.x = point.x;
	e.y = point.y;

	Window::GetInstance().QueueEvent( e );
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
