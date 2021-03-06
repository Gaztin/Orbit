/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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

#include <cmath>

#if defined( ORB_OS_WINDOWS )
#  include <windowsx.h>
#elif defined( ORB_OS_MACOS ) // ORB_OS_WINDOWS
#  include <AppKit/AppKit.h>
#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS
#  include <android/sensor.h>
#endif // ORB_OS_ANDROID

ORB_NAMESPACE_BEGIN

#if defined( ORB_OS_WINDOWS )
static ATOM RegisterWindowClass( LPCSTR name );
#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS
static void HandleXEvent( Window* w, const XEvent& xevent );
#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX
static void HandleNSEvent( NSEvent* nsevent, NSWindow* nswindow );
#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS
static void AppCMD ( AndroidApp* state, AndroidAppCommand cmd );
static int  OnInput( AndroidApp* state, AInputEvent* e );
#endif // ORB_OS_ANDROID

Window::Window( uint32_t width, uint32_t height )
	: details_{ }
	, open_   { true }
{

#if defined( ORB_OS_WINDOWS )

	constexpr LPCSTR class_name   = "OrbitWindow";
	static ATOM      window_class = RegisterWindowClass( class_name );

	/* Create window */
	details_.hwnd = CreateWindowExA( WS_EX_OVERLAPPEDWINDOW, class_name, NULL, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandleA( NULL ), NULL );

	/* Set user data */
	SetWindowLongPtrA( details_.hwnd, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( this ) );

#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

	/* Open display */
	details_.display = XOpenDisplay( nullptr );

	/* Create window */
	const int               screen      = DefaultScreen( details_.display );
	XID                     root_window = XRootWindow( details_.display, screen );
	int                     depth       = DefaultDepth( details_.display, screen );
	Visual*                 visual      = DefaultVisual( details_.display, screen );
	constexpr unsigned long value_mask  = ( CWBackPixel | CWEventMask );
	XSetWindowAttributes    attribs     = { };
	attribs.event_mask                  = ( FocusChangeMask | ResizeRedirectMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask );
	details_.window                     = XCreateWindow( details_.display, root_window, 0, 0, width, height, 0, depth, InputOutput, visual, value_mask, &attribs );

	/* Allows us to capture the window close event */
	details_.wm_delete_window = XInternAtom( details_.display, "WM_DELETE_WINDOW", True );
	XSetWMProtocols( details_.display, details_.window, &details_.wm_delete_window, 1 );
	
	/* Send initial resize event */
	XEvent xevent;
	xevent.xresizerequest.type       = ResizeRequest;
	xevent.xresizerequest.display    = details_.display;
	xevent.xresizerequest.window     = details_.window;
	xevent.xresizerequest.width      = width;
	xevent.xresizerequest.height     = height;
	xevent.xresizerequest.send_event = True;
	xevent.xresizerequest.serial     = 0;
	XSendEvent( details_.display, details_.window, False, 0, &xevent );

#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

	NSRect             frame      = NSMakeRect( 0.0f, 0.0f, width, height );
	NSWindowStyleMask  style_mask = ( NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable );
	NSBackingStoreType backing    = ( NSBackingStoreBuffered );

	/* Create window */
	details_.window = [ NSWindow alloc ];
	[ details_.window initWithContentRect:frame styleMask:style_mask backing:backing defer:NO ];

	/* Create window delegate */
	details_.delegate = [ OrbitWindowDelegate alloc ];
	[ details_.window setDelegate:details_.delegate ];

	/* Send initial resize event */
	WindowResized e;
	e.width  = width;
	e.height = height;
	QueueEvent( e );

#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

	( void )width;
	( void )height;

	AndroidOnly::app->on_input_event = OnInput;

	details_.sensor_manager       = ASensorManager_getInstance();
	details_.accelerometer_sensor = ASensorManager_getDefaultSensor( details_.sensor_manager, ASENSOR_TYPE_ACCELEROMETER );
	details_.sensor_event_queue   = ASensorManager_createEventQueue( details_.sensor_manager, AndroidOnly::app->looper, static_cast< int >( AndroidLooperID::User ), nullptr, nullptr );

	AndroidOnly::app->user_data  = this;
	AndroidOnly::app->on_app_cmd = AppCMD;

	/* Update until native window is initialized */
	while( !AndroidOnly::app->window )
	{
		AndroidPollSource* source;

		if( ALooper_pollAll( 0, nullptr, nullptr, reinterpret_cast< void** >( &source ) ) >= 0 && source )
			source->process( AndroidOnly::app, source );
	}

#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

	( void )width;
	( void )height;

	/* Initialize window */
	details_.ui_window = [ OrbitUIWindow alloc ];
	[ details_.ui_window initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
	details_.ui_window.backgroundColor = [ UIColor whiteColor ];
	[ details_.ui_window makeKeyAndVisible ];
	[ details_.ui_window setMultipleTouchEnabled:YES ];

	/* Create view controller */
	UIViewController* vc = [ UIViewController alloc ];
	[ vc initWithNibName:nil bundle:nil ];
	details_.ui_window.rootViewController = vc;

#endif // ORB_OS_IOS

}

Window::~Window( void )
{

#if defined( ORB_OS_WINDOWS )

	DestroyWindow( details_.hwnd );

#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

	XDestroyWindow( details_.display, details_.window );
	XCloseDisplay( details_.display );

#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

	[ details_.window close ];
	[ details_.delegate dealloc ];
	[ details_.window dealloc ];

#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

	ASensorManager_destroyEventQueue( details_.sensor_manager, details_.sensor_event_queue );
	AndroidOnly::app->user_data  = nullptr;
	AndroidOnly::app->on_app_cmd = nullptr;

#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

	[ details_.ui_window dealloc ];

#endif // ORB_OS_IOS

}

void Window::PollEvents( void )
{
	Input::ResetStates();

#if defined( ORB_OS_WINDOWS )

	MSG msg;

	while( PeekMessageA( &msg, details_.hwnd, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}

#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

	while( XPending( details_.display ) )
	{
		XEvent xevent;
		XNextEvent( details_.display, &xevent );

		HandleXEvent( this, xevent );
	}

#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

	NSEvent* event;

	while( ( event = [ details_.window nextEventMatchingMask:NSEventMaskAny untilDate:nullptr inMode:NSDefaultRunLoopMode dequeue:YES ] ) != nullptr )
	{
		HandleNSEvent( event, details_.window );

		[ details_.window sendEvent:event ];
	}

#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

	AndroidPollSource* source;

	if( ALooper_pollAll( 0, nullptr, nullptr, reinterpret_cast< void** >( &source ) ) >= 0 && source )
		source->process( AndroidOnly::app, source );

#endif // ORB_OS_ANDROID

	/* Send events */
	SendEvents();
}

void Window::SetTitle( std::string_view title )
{

#if defined( ORB_OS_WINDOWS )

	SetWindowTextA( details_.hwnd, title.data() );

#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

	XStoreName( details_.display, details_.window, title.data() );

#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

	NSString* title_objc = [ NSString stringWithUTF8String:title.data() ];
	[ details_.window setTitle:title_objc ];
	[ title_objc release ];

#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

	// #TODO: Activity.setTitle
	( void )title;

#endif // ORB_OS_ANDROID

}

void Window::Move( int32_t x, int32_t y )
{

#if defined( ORB_OS_WINDOWS )

	RECT rect;

	if( GetWindowRect( details_.hwnd, &rect ) )
	{
		const int width  = ( rect.right - rect.left );
		const int height = ( rect.bottom - rect.top );

		MoveWindow( details_.hwnd, x, y, width, height, FALSE );
	}

#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

	XMoveWindow( details_.display, details_.window, x, y );

#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

	NSRect frame   = [ details_.window frame ];
	frame.origin.x = x;
	frame.origin.y = y;

	[ details_.window setFrame:frame display:YES ];

#else // ORB_OS_MACOS

	( void )x;
	( void )y;

#endif // !ORB_OS_MACOS

}

void Window::Resize( uint32_t width, uint32_t height )
{

#if defined( ORB_OS_WINDOWS )

	RECT rect;
	
	if( GetWindowRect( details_.hwnd, &rect ) )
		MoveWindow( details_.hwnd, rect.left, rect.top, width, height, TRUE );

#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

	XResizeWindow( details_.display, details_.window, width, height );

#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

	NSRect frame      = [ details_.window frame ];
	frame.size.width  = width;
	frame.size.height = height;

	[ details_.window setFrame:frame display:YES ];

#else // ORB_OS_MACOS

	( void )width;
	( void )height;

#endif // !ORB_OS_MACOS

}

void Window::Show( void )
{

#if defined( ORB_OS_WINDOWS )

	ShowWindow( details_.hwnd, SW_SHOW );

#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

	XMapWindow( details_.display, details_.window );

#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

	[ details_.window setIsVisible:YES ];

#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

	// #TODO: Activity.setVisible

#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

	[ ( OrbitUIWindow* )details_.ui_window setHidden:NO ];

#endif // ORB_OS_IOS

}

void Window::Hide( void )
{

#if defined( ORB_OS_WINDOWS )

	ShowWindow( details_.hwnd, SW_HIDE );

#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

	XUnmapWindow( details_.display, details_.window );

#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

	[ details_.window setIsVisible:NO ];

#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

	// #TODO: Activity.setVisible

#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

	[ details_.ui_window setHidden:YES ];

#endif // ORB_OS_IOS

}

void Window::Close( void )
{
	open_ = false;
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
			Input::SetPointerPressed( Input::pointer_index_mouse_left, pos );

			break;
		}

		case WM_LBUTTONUP:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerReleased( Input::pointer_index_mouse_left, pos );

			break;
		}

		case WM_MBUTTONDOWN:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerPressed( Input::pointer_index_mouse_middle, pos );

			break;
		}

		case WM_MBUTTONUP:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerReleased( Input::pointer_index_mouse_middle, pos );

			break;
		}

		case WM_RBUTTONDOWN:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerPressed( Input::pointer_index_mouse_right, pos );

			break;
		}

		case WM_RBUTTONUP:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
			Input::SetPointerReleased( Input::pointer_index_mouse_right, pos );

			break;
		}

		case WM_XBUTTONDOWN:
		{
			WORD  which = HIWORD( wparam );
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );

			switch( which )
			{
				case XBUTTON1: { Input::SetPointerPressed( Input::pointer_index_mouse_extra_1, pos ); } break;
				case XBUTTON2: { Input::SetPointerPressed( Input::pointer_index_mouse_extra_2, pos ); } break;
			}

			break;
		}

		case WM_XBUTTONUP:
		{
			WORD  which = HIWORD( wparam );
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );

			switch( which )
			{
				case XBUTTON1: { Input::SetPointerReleased( Input::pointer_index_mouse_extra_1, pos ); } break;
				case XBUTTON2: { Input::SetPointerReleased( Input::pointer_index_mouse_extra_2, pos ); } break;
			}

			break;
		}

		case WM_MOUSEMOVE:
		{
			Point pos( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );

			/* FIXME: Mouse pos won't be tracked until any mouse button has been pressed */
			for( size_t index : Input::GetPointerIndices() )
				Input::SetPointerPos( index, pos );

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
			auto& details = w->GetPrivateDetails();
			Atom  atom    = *reinterpret_cast< const Atom* >( xevent.xclient.data.l );
			
			if( atom == details.wm_delete_window )
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
		
		case ButtonPress:
		{
			Point pos( xevent.xbutton.x, xevent.xbutton.y );

			switch( xevent.xbutton.button )
			{
				case Button1: { Input::SetPointerPressed( Input::pointer_index_mouse_left, pos );    } break;
				case Button2: { Input::SetPointerPressed( Input::pointer_index_mouse_middle, pos );  } break;
				case Button3: { Input::SetPointerPressed( Input::pointer_index_mouse_right, pos );   } break;
				case Button4: { Input::SetPointerPressed( Input::pointer_index_mouse_extra_1, pos ); } break;
				case Button5: { Input::SetPointerPressed( Input::pointer_index_mouse_extra_2, pos ); } break;
			}

			break;
		}
		
		case ButtonRelease:
		{
			Point pos( xevent.xbutton.x, xevent.xbutton.y );

			switch( xevent.xbutton.button )
			{
				case Button1: { Input::SetPointerReleased( Input::pointer_index_mouse_left, pos );    } break;
				case Button2: { Input::SetPointerReleased( Input::pointer_index_mouse_middle, pos );  } break;
				case Button3: { Input::SetPointerReleased( Input::pointer_index_mouse_right, pos );   } break;
				case Button4: { Input::SetPointerReleased( Input::pointer_index_mouse_extra_1, pos ); } break;
				case Button5: { Input::SetPointerReleased( Input::pointer_index_mouse_extra_2, pos ); } break;
			}
			
			break;
		}
		
		case MotionNotify:
		{
			Point pos( xevent.xmotion.x, xevent.xmotion.y );

			/* FIXME: Mouse pos won't be tracked until any mouse button has been pressed */
			for( size_t index : Input::GetPointerIndices() )
				Input::SetPointerPos( index, pos );

			break;
		}
	}
}

#elif defined( ORB_OS_MACOS )

void HandleNSEvent( NSEvent* nsevent, NSWindow* nswindow )
{
	switch( nsevent.type )
	{
		default: break;

		case NSEventTypeKeyDown:
		{
			Key key = ConvertSystemKey( nsevent.keyCode );
			Input::SetKeyPressed( key );

			break;
		}

		case NSEventTypeKeyUp:
		{
			Key key = ConvertSystemKey( nsevent.keyCode );
			Input::SetKeyReleased( key );

			break;
		}

		case NSEventTypeLeftMouseDown:
		{
			NSPoint relative_mouse_pos = [ nswindow convertPointFromScreen:NSEvent.mouseLocation ];
			Point   pos( relative_mouse_pos.x, relative_mouse_pos.y );

			Input::SetPointerPressed( Input::pointer_index_mouse_left, pos );

			break;
		}

		case NSEventTypeLeftMouseUp:
		{
			NSPoint relative_mouse_pos = [ nswindow convertPointFromScreen:NSEvent.mouseLocation ];
			Point   pos( relative_mouse_pos.x, relative_mouse_pos.y );

			Input::SetPointerReleased( Input::pointer_index_mouse_left, pos );

			break;
		}

		case NSEventTypeRightMouseDown:
		{
			NSPoint relative_mouse_pos = [ nswindow convertPointFromScreen:NSEvent.mouseLocation ];
			Point   pos( relative_mouse_pos.x, relative_mouse_pos.y );

			Input::SetPointerPressed( Input::pointer_index_mouse_right, pos );

			break;
		}

		case NSEventTypeRightMouseUp:
		{
			NSPoint relative_mouse_pos = [ nswindow convertPointFromScreen:NSEvent.mouseLocation ];
			Point   pos( relative_mouse_pos.x, relative_mouse_pos.y );

			Input::SetPointerReleased( Input::pointer_index_mouse_right, pos );

			break;
		}

		case NSEventTypeOtherMouseDown:
		{
			NSPoint relative_mouse_pos = [ nswindow convertPointFromScreen:NSEvent.mouseLocation ];
			Point   pos( relative_mouse_pos.x, relative_mouse_pos.y );

			Input::SetPointerPressed( Input::pointer_index_mouse_middle, pos );

			break;
		}

		case NSEventTypeOtherMouseUp:
		{
			NSPoint relative_mouse_pos = [ nswindow convertPointFromScreen:NSEvent.mouseLocation ];
			Point   pos( relative_mouse_pos.x, relative_mouse_pos.y );

			Input::SetPointerReleased( Input::pointer_index_mouse_middle, pos );

			break;
		}

		case NSEventTypeMouseMoved:
		{
			NSPoint relative_mouse_pos = [ nswindow convertPointFromScreen:NSEvent.mouseLocation ];
			Point   pos( relative_mouse_pos.x, relative_mouse_pos.y );

			/* FIXME: Mouse pos won't be tracked until any mouse button has been pressed */
			for( size_t index : Input::GetPointerIndices() )
				Input::SetPointerPos( index, pos );

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

		case AINPUT_EVENT_TYPE_KEY:
		{
			Key key = ConvertSystemKey( AKeyEvent_getKeyCode( e ) );

			switch( AKeyEvent_getAction( e ) )
			{
				default: break;

				case AKEY_EVENT_ACTION_DOWN:
				{
					Input::SetKeyPressed( key );
					break;
				}

				case AKEY_EVENT_ACTION_UP:
				{
					Input::SetKeyReleased( key );
					break;
				}
			}

			break;
		}

		case AINPUT_EVENT_TYPE_MOTION:
		{
			uint32_t action_and_pointer_index = static_cast< uint32_t >( AMotionEvent_getAction( e ) );
			uint32_t action                   = ( action_and_pointer_index & 0xff );
			uint32_t pointer_index            = ( action_and_pointer_index & 0xffffff00 ) >> 8;
			int32_t x                         = static_cast< int32_t >( std::round( AMotionEvent_getX( e, pointer_index ) ) );
			int32_t y                         = static_cast< int32_t >( std::round( AMotionEvent_getY( e, pointer_index ) ) );

			switch( action )
			{
				default: break;

				case AMOTION_EVENT_ACTION_DOWN:
				case AMOTION_EVENT_ACTION_POINTER_DOWN:
				{
					Input::SetPointerPressed( pointer_index, Point( x, y ) );
					break;
				}

				case AMOTION_EVENT_ACTION_CANCEL:
				case AMOTION_EVENT_ACTION_UP:
				case AMOTION_EVENT_ACTION_POINTER_UP:
				{
					Input::SetPointerReleased( pointer_index, Point( x, y ) );
					break;
				}

				case AMOTION_EVENT_ACTION_MOVE:
				{
					Input::SetPointerPos( pointer_index, Point( x, y ) );
					break;
				}
			}

			break;
		}
	}

	return 0;
};

#endif

ORB_NAMESPACE_END
