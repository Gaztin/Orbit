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

#if _ORB_HAS_WINDOW_API_COCOA
#  include <AppKit/AppKit.h>
@interface OrbitWindowDelegate : NSObject< NSWindowDelegate >
@property ORB_NAMESPACE Window*     window_ptr;
@property ORB_NAMESPACE WindowImpl* impl;
@end
#endif

#if _ORB_HAS_WINDOW_API_UIKIT
#  include <UIKit/UIKit.h>
@interface OrbitUIWindow : UIWindow
@property ORB_NAMESPACE Window* window_ptr;
@end
#endif

ORB_NAMESPACE_BEGIN

template< typename T >
constexpr auto window_impl_index_v = unique_index_v< T, WindowImpl >;

Window::Window( [[ maybe_unused ]] uint32_t width, [[ maybe_unused ]] uint32_t height, WindowAPI api )
	: m_impl { }
	, m_open { true }
{
	switch( api )
	{
		default: break;

	#if _ORB_HAS_WINDOW_API_WIN32
		case WindowAPI::Win32:
		{
			auto             impl         = std::addressof( m_impl.emplace< _WindowImplWin32 >() );
			constexpr LPCSTR kClassName   = "OrbitWindow";
			static ATOM      window_class = [ & ]
			{
				auto window_proc = []( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
				{
					Window* w = reinterpret_cast< Window* >( GetWindowLongPtrA( hwnd, GWLP_USERDATA ) );
					if( w == nullptr )
						return DefWindowProcA( hwnd, msg, wparam, lparam );

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
				};

				WNDCLASSEXA classDesc{ };
				classDesc.cbSize        = sizeof( WNDCLASSEXA );
				classDesc.style         = CS_VREDRAW | CS_HREDRAW;
				classDesc.lpfnWndProc   = window_proc;
				classDesc.hInstance     = GetModuleHandleA( nullptr );
				classDesc.hbrBackground = reinterpret_cast< HBRUSH >( COLOR_WINDOW );
				classDesc.lpszClassName = kClassName;

				/* Extract and copy icon from application. */
				char path[ MAX_PATH + 1 ];
				GetModuleFileNameA( nullptr, path, sizeof( path ) );
				classDesc.hIcon = ExtractIconA( classDesc.hInstance, path, 0 );

				return RegisterClassExA( &classDesc );
			}();

			/* Create window */
			impl->hwnd = CreateWindowExA( WS_EX_OVERLAPPEDWINDOW, kClassName, NULL, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandleA( NULL ), NULL );

			/* Set user data */
			SetWindowLongPtrA( impl->hwnd, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( this ) );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_X11
		case WindowAPI::X11:
		{
			/* Open display */
			auto impl     = std::addressof( m_impl.emplace< _WindowImplX11 >() );
			impl->display = XOpenDisplay( nullptr );

			/* Create window */
			const int               screen      = DefaultScreen( impl->display );
			Window                  root_window = XRootWindow( impl->display, screen );
			int                     depth       = DefaultDepth( impl->display, screen );
			Visual*                 visual      = DefaultVisual( impl->display, screen );
			constexpr unsigned long kValueMask  = ( CWBackPixel | CWEventMask );
			XSetWindowAttributes    attribs     = { };
			attribs.event_mask                  = ( FocusChangeMask | ResizeRedirectMask | StructureNotifyMask );
			impl->window                        = XCreateWindow( impl->display, root_window, 0, 0, width, height, 0, depth, InputOutput, visual, kValueMask, &attribs );

			/* Allow us to capture the window close event */
			Atom close_atom = XInternAtom( impl->display, "WM_DELETE_WINDOW", True );
			XSetWMProtocols( impl->display, impl->window, &close_atom, 1 );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_WAYLAND
		case WindowAPI::Wayland:
		{
			m_impl.emplace< _WindowImplWayland >();

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_COCOA
		case WindowAPI::Cocoa:
		{
			auto               impl       = std::addressof( m_impl.emplace< _WindowImplCocoa >() );
			NSRect             frame      = NSMakeRect( 0.0f, 0.0f, width, height );
			NSWindowStyleMask  style_mask = ( NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable );
			NSBackingStoreType backing    = ( NSBackingStoreBuffered );

			/* Create window */
			impl->ns_window = [ NSWindow alloc ];
			[ ( NSWindow* )impl->ns_window initWithContentRect:frame styleMask:style_mask backing:backing defer:NO ];

			/* Create window delegate */
			impl->delegate = [ OrbitWindowDelegate alloc ];
			[ ( NSWindow* )impl->ns_window setDelegate:( OrbitWindowDelegate* )impl->delegate ];
			[ ( OrbitWindowDelegate* )impl->delegate setWindowPtr:this ];
			[ ( OrbitWindowDelegate* )impl->delegate setImpl:&m_impl ];

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_ANDROID
		case WindowAPI::Android:
		{
			auto app_cmd = []( android_app* state, int cmd )
			{
				Window* w = static_cast< Window* >( state->userData );

				switch( cmd )
				{
					default: break;

					case APP_CMD_INIT_WINDOW:
					{
						WindowEvent resize_event { WindowEvent::Resize };
						resize_event.data.resize.w = static_cast< uint32_t >( ANativeWindow_getWidth( state->window ) );
						resize_event.data.resize.h = static_cast< uint32_t >( ANativeWindow_getHeight( state->window ) );

						w->QueueEvent( resize_event );
						w->QueueEvent( { WindowEvent::Restore } );

						break;
					}

					case APP_CMD_TERM_WINDOW:
					{
						w->QueueEvent( { WindowEvent::Suspend } );

						break;
					}

					case APP_CMD_GAINED_FOCUS:
					{
						auto impl = std::get_if< _WindowImplAndroid >( w->GetImplPtr() );
						ASensorEventQueue_enableSensor( impl->sensor_event_queue, impl->accelerometer_sensor );
						ASensorEventQueue_setEventRate( impl->sensor_event_queue, impl->accelerometer_sensor, ( 1000 * 1000 / 60 ) );

						w->QueueEvent( { WindowEvent::Focus } );

						break;
					}

					case APP_CMD_LOST_FOCUS:
					{
						auto impl = std::get_if< _WindowImplAndroid >( w->GetImplPtr() );
						ASensorEventQueue_disableSensor( impl->sensor_event_queue, impl->accelerometer_sensor );

						w->QueueEvent( { WindowEvent::Defocus } );

						break;
					}

					case APP_CMD_DESTROY:
					{
						w->Close();
						break;
					}
				}
			};

			auto on_input = []( android_app* state, AInputEvent* e )
			{
				switch( AInputEvent_getType( e ) )
				{
					default: break;
				}

				return 0;
			};

			AndroidOnly::app->onInputEvent = on_input;

			auto impl                  = std::addressof( m_impl.emplace< _WindowImplAndroid >() );
			impl->sensor_manager       = ASensorManager_getInstance();
			impl->accelerometer_sensor = ASensorManager_getDefaultSensor( impl->sensor_manager, ASENSOR_TYPE_ACCELEROMETER );
			impl->sensor_event_queue   = ASensorManager_createEventQueue( impl->sensor_manager, AndroidOnly::app->looper, LOOPER_ID_USER, nullptr, nullptr );

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
			AndroidOnly::app->onAppCmd = appCmd;

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_UIKIT
		case WindowAPI::UiKit:
		{
			auto impl = std::addressof( m_impl.emplace< _WindowImplUIKit >() );

			/* Initialize window */
			impl->ui_window = [ OrbitUIWindow alloc ];
			[ ( OrbitUIWindow* )impl->ui_window initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
			( ( OrbitUIWindow* )impl->ui_window ).backgroundColor = [ UIColor whiteColor ];
			[ ( OrbitUIWindow* )impl->ui_window makeKeyAndVisible];
			[ ( OrbitUIWindow* )impl->ui_window setWindowPtr:this ];

			/* Create view controller */
			UIViewController* vc = [ UIViewController alloc ];
			[ vc initWithNibName:nil bundle:nil ];
			( ( OrbitUIWindow* )impl->ui_window ).rootViewController = vc;

			break;
		}
	#endif
	}
}

Window::~Window()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_WINDOW_API_WIN32
		case( window_impl_index_v< _WindowImplWin32 > ):
		{
			auto impl = std::get_if< _WindowImplWin32 >( &m_impl );
			DestroyWindow( impl->hwnd );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_X11
		case( window_impl_index_v< _WindowImplX11 > ):
		{
			auto impl = std::get_if< _WindowImplX11 >( &m_impl );
			XDestroyWindow( impl->display, impl->window );
			XCloseDisplay( impl->display );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_WAYLAND
		case( window_impl_index_v< _WindowImplWayland > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_COCOA
		case( window_impl_index_v< _WindowImplCocoa > ):
		{
			auto impl = std::get_if< _WindowImplCocoa >( &m_impl );
			[ ( NSWindow* )impl->ns_window close ];
			[ ( OrbitWindowDelegate* )impl->delegate dealloc ];
			[ ( NSWindow* )impl->nsWindow dealloc ];

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_ANDROID
		case( window_impl_index_v< _WindowImplAndroid > ):
		{
			auto impl = std::get_if< _WindowImplAndroid >( &m_impl );
			ASensorManager_destroyEventQueue( impl->sensor_manager, impl->sensor_event_queue );
			AndroidOnly::app->userData = nullptr;
			AndroidOnly::app->onAppCmd = nullptr;

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_UIKIT
		case( window_impl_index_v< _WindowImplUIKit > ):
		{
			auto impl = std::get_if< _WindowImplUIKit >( &m_impl );
			[ ( OrbitUIWindow* )impl->ui_window dealloc ];

			break;
		}
	#endif
	}
}

void Window::PollEvents()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_WINDOW_API_WIN32
		case( window_impl_index_v< _WindowImplWin32 > ):
		{
			auto impl = std::get_if< _WindowImplWin32 >( &m_impl );

			MSG msg;
			while( PeekMessageA( &msg, impl->hwnd, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessageA( &msg );
			}

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_X11
		case( window_impl_index_v< _WindowImplX11 > ):
		{
			auto impl = std::get_if< _WindowImplX11 >( &m_impl );

			while( XPending( impl->display ) )
			{
				XEvent xevent;
				XNextEvent( impl->display, &xevent );

				switch( xevent.type )
				{
					default: break;

					case FocusIn:
					{
						if( xevent.xfocus.mode != NotifyNormal )
							break;

						WindowEvent e { };
						e.type = WindowEvent::Focus;
						QueueEvent( e );

						break;
					}

					case FocusOut:
					{
						if( xevent.xfocus.mode != NotifyNormal )
							break;

						WindowEvent e { };
						e.type = WindowEvent::Defocus;
						QueueEvent( e );

						break;
					}

					case ResizeRequest:
					{
						WindowEvent e { };
						e.type          = WindowEvent::Resize;
						e.data.resize.w = xevent.xresizerequest.width;
						e.data.resize.h = xevent.xresizerequest.height;
						QueueEvent( e );

						break;
					}

					case ConfigureNotify:
					{
						WindowEvent e { };
						e.type        = WindowEvent::Move;
						e.data.move.x = xevent.xconfigure.x;
						e.data.move.y = xevent.xconfigure.y;
						QueueEvent( e );

						break;
					}

					case ClientMessage:
					{
						Close();
						break;
					}
				}
			}

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_WAYLAND
		case( window_impl_index_v< _WindowImplWayland > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_COCOA
		case( window_impl_index_v< _WindowImplCocoa > ):
		{
			auto     impl = std::get_if< _WindowImplCocoa >( &m_impl );
			NSEvent* ns_event;
			while( ( ns_event = [ ( const NSWindow* )impl->ns_window nextEventMatchingMask:NSEventMaskAny untilDate:nullptr inMode:NSDefaultRunLoopMode dequeue:YES ] ) != nullptr )
			{
				[ ( const NSWindow* )impl->ns_window sendEvent:ns_event ];
			}

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_ANDROID
		case( window_impl_index_v< _WindowImplAndroid > ):
		{
			int                  events;
			android_poll_source* source;

			if( ALooper_pollAll( 0, nullptr, &events, reinterpret_cast< void** >( &source ) ) >= 0 )
			{
				if( source )
					source->process( AndroidOnly::app, source );
			}

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_UIKIT
		case( window_impl_index_v< _WindowImplUIKit > ):
		{
			break;
		}
	#endif
	}

	/* Send events */
	SendEvents();
}

void Window::SetTitle( [[ maybe_unused ]] std::string_view title )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_WINDOW_API_WIN32
		case( window_impl_index_v< _WindowImplWin32 > ):
		{
			auto impl = std::get_if< _WindowImplWin32 >( &m_impl );
			SetWindowTextA( impl->hwnd, title.data() );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_X11
		case( window_impl_index_v< _WindowImplX11 > ):
		{
			auto impl = std::get_if< _WindowImplX11 >( &m_impl );
			XStoreName( impl->display, impl->window, title.data() );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_WAYLAND
		case( window_impl_index_v< _WindowImplWayland > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_COCOA
		case( window_impl_index_v< _WindowImplCocoa > ):
		{
			auto      impl     = std::get_if< _WindowImplCocoa >( &m_impl );
			NSString* ns_title = [ NSString stringWithUTF8String:title.data() ];
			[ ( const NSWindow* )impl->ns_window setTitle:ns_title ];
			[ nsTitle release ];

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_ANDROID
		case( window_impl_index_v< _WindowImplAndroid > ):
		{
			// #TODO: Activity.setTitle
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_UIKIT
		case( window_impl_index_v< _WindowImplUIKit > ):
		{
			break;
		}
	#endif
	}
}

void Window::SetPos( [[ maybe_unused ]] uint32_t x, [[ maybe_unused ]] uint32_t y )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_WINDOW_API_WIN32
		case( window_impl_index_v< _WindowImplWin32 > ):
		{
			auto impl = std::get_if< _WindowImplWin32 >( &m_impl );

			RECT rect { };
			GetWindowRect( impl->hwnd, &rect );
			MoveWindow( impl->hwnd, x, y, ( rect.right - rect.left ), ( rect.bottom - rect.top ), FALSE );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_X11
		case( window_impl_index_v< _WindowImplX11 > ):
		{
			auto impl = std::get_if< _WindowImplX11 >( &m_impl );
			XMoveWindow( impl->display, impl->window, x, y );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_WAYLAND
		case( window_impl_index_v< _WindowImplWayland > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_COCOA
		case( window_impl_index_v< _WindowImplCocoa > ):
		{
			auto   impl    = std::get_if< _WindowImplCocoa >( &m_impl );
			NSRect frame   = [ ( const NSWindow* )impl->ns_window frame ];
			frame.origin.x = x;
			frame.origin.y = y;
			[ ( const NSWindow* )impl->ns_window setFrame:frame display:YES ];

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_ANDROID
		case( window_impl_index_v< _WindowImplAndroid > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_UIKIT
		case( window_impl_index_v< _WindowImplUIKit > ):
		{
			break;
		}
	#endif
	}
}

void Window::SetSize( [[ maybe_unused ]] uint32_t width, [[ maybe_unused ]] uint32_t height )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_WINDOW_API_WIN32
		case( window_impl_index_v< _WindowImplWin32 > ):
		{
			auto impl = std::get_if< _WindowImplWin32 >( &m_impl );

			RECT rect { };
			GetWindowRect( impl->hwnd, &rect );
			MoveWindow( impl->hwnd, rect.left, rect.top, static_cast< int >( width ), static_cast< int >( height ), FALSE );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_X11
		case( window_impl_index_v< _WindowImplX11 > ):
		{
			auto impl = std::get_if< _WindowImplX11 >( &m_impl );
			XResizeWindow( impl->display, impl->window, width, height );
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_WAYLAND
		case( window_impl_index_v< _WindowImplWayland > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_COCOA
		case( window_impl_index_v< _WindowImplCocoa > ):
		{
			auto   impl       = std::get_if< _WindowImplCocoa >( &m_impl );
			NSRect frame      = [ ( const NSWindow* )impl->ns_window frame ];
			frame.size.width  = width;
			frame.size.height = height;
			[ ( const NSWindow* )impl->ns_window setFrame:frame display:YES ];

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_ANDROID
		case( window_impl_index_v< _WindowImplAndroid > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_UIKIT
		case( window_impl_index_v< _WindowImplUIKit > ):
		{
			break;
		}
	#endif
	}
}

void Window::Show()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_WINDOW_API_WIN32
		case( window_impl_index_v< _WindowImplWin32 > ):
		{
			auto impl = std::get_if< _WindowImplWin32 >( &m_impl );
			ShowWindow( impl->hwnd, SW_SHOW );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_X11
		case( window_impl_index_v< _WindowImplX11 > ):
		{
			auto impl = std::get_if< _WindowImplX11 >( &m_impl );
			XMapWindow( impl->display, impl->window );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_WAYLAND
		case( window_impl_index_v< _WindowImplWayland > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_COCOA
		case( window_impl_index_v< _WindowImplCocoa > ):
		{
			auto impl = std::get_if< _WindowImplCocoa >( &m_impl );
			[ ( const NSWindow* )impl->ns_window setIsVisible:YES ];

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_ANDROID
		case( window_impl_index_v< _WindowImplAndroid > ):
		{
			// #TODO: Activity.setVisible
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_UIKIT
		case( window_impl_index_v< _WindowImplUIKit > ):
		{
			auto impl = std::get_if< _WindowImplUIKit >( &m_impl );
			[ ( OrbitUIWindow* )impl->ui_window setHidden:NO ];

			break;
		}
	#endif
	}
}

void Window::Hide()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_WINDOW_API_WIN32
		case( window_impl_index_v< _WindowImplWin32 > ):
		{
			auto impl = std::get_if< _WindowImplWin32 >( &m_impl );
			ShowWindow( impl->hwnd, SW_HIDE );

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_X11
		case( window_impl_index_v< _WindowImplX11 > ):
		{
			auto impl = std::get_if< _WindowImplX11 >( &m_impl );
			XUnmapWindow( impl->display, impl->window );
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_WAYLAND
		case( window_impl_index_v< _WindowImplWayland > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_COCOA
		case( window_impl_index_v< _WindowImplCocoa > ):
		{
			auto impl = std::get_if< _WindowImplCocoa >( &m_impl );
			[ ( const NSWindow* )impl->ns_window setIsVisible:NO ];

			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_ANDROID
		case( window_impl_index_v< _WindowImplAndroid > ):
		{
			break;
		}
	#endif

	#if _ORB_HAS_WINDOW_API_UIKIT
		case( window_impl_index_v< _WindowImplUIKit > ):
		{
			auto impl = std::get_if< _WindowImplUIKit >( &m_impl );
			[ ( OrbitUIWindow* )impl->ui_window setHidden:YES ];

			break;
		}
	#endif
	}
}

ORB_NAMESPACE_END

#if _ORB_HAS_WINDOW_API_COCOA

@implementation OrbitWindowDelegate

-( void )windowWillClose:( NSNotification* ) __unused notification
{
	_windowPtr->close();
}

-( void )windowDidMove:( NSNotification* ) __unused notification
{
	auto          impl  = std::get_if< ORB_NAMESPACE _WindowImplCocoa >( _impl );
	const CGPoint point = ( ( const NSWindow* )impl->ns_window ).frame.origin;

	ORB_NAMESPACE WindowEvent e { };
	e.type        = ORB_NAMESPACE WindowEvent::Move;
	e.data.move.x = static_cast< int >( point.x );
	e.data.move.y = static_cast< int >( point.y );
	_window_ptr->QueueEvent( e );
}

-( NSSize )windowWillResize:( NSWindow* ) __unused sender toSize:( NSSize ) frameSize
{
	ORB_NAMESPACE WindowEvent e { };
	e.type          = ORB_NAMESPACE WindowEvent::Resize;
	e.data.resize.w = static_cast< uint32_t >( frameSize.width );
	e.data.resize.h = static_cast< uint32_t >( frameSize.height );
	_window_ptr->QueueEvent( e );

	return frameSize;
}

-( void )windowDidMiniaturize:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowEvent e { };
	e.type = ORB_NAMESPACE WindowEvent::Suspend;
	_window_ptr->QueueEvent( e );
}

-( void )windowDidDeminiaturize:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowEvent e { };
	e.type = ORB_NAMESPACE WindowEvent::Restore;
	_window_ptr->QueueEvent( e );
}

-( void )windowDidBecomeMain:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowEvent e { };
	e.type = ORB_NAMESPACE WindowEvent::Focus;
	_window_ptr->QueueEvent( e );
}

-( void )windowDidResignMain:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowEvent e { };
	e.type = ORB_NAMESPACE WindowEvent::Defocus;
	_window_ptr->QueueEvent( e );
}

@end

#endif

#if _ORB_HAS_WINDOW_API_UIKIT

@implementation OrbitUIWindow

-( void )layoutSubviews
{
	[ super layoutSubviews ];

	ORB_NAMESPACE WindowEvent e { };
	e.type          = ORB_NAMESPACE WindowEvent::Resize;
	e.data.resize.w = static_cast< uint32_t >( self.bounds.size.width );
	e.data.resize.h = static_cast< uint32_t >( self.bounds.size.height );
	_window_ptr->QueueEvent( e );
}

@end

#endif
