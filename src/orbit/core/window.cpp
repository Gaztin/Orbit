/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "window.h"

#include "orbit/core/utility.h"

#if __ORB_HAS_WINDOW_IMPL_COCOA
@interface ORBCocoaWindowDelegate : NSObject< NSWindowDelegate >
@property orb::window*      windowPtr;
@property orb::window_impl* impl;
@end
#endif

#if __ORB_HAS_WINDOW_IMPL_UIKIT
@interface ORBUiKitWindow : UIWindow
@property orb::window* windowPtr;
@end
#endif

namespace orb
{
	template< typename T >
	constexpr auto window_impl_index_v = unique_index_v< T, window_impl >;

	window::window( uint32_t width, uint32_t height, window_impl_type implType )
		: m_impl{ }
		, m_open( true )
	{
		switch( implType )
		{
			default:
			{
				( void )width;
				( void )height;
				break;
			}

	#if __ORB_HAS_WINDOW_IMPL_WIN32
			case window_impl_type::Win32:
			{
				auto             impl         = std::addressof( m_impl.emplace< __window_impl_win32 >() );
				constexpr LPCSTR kClassName   = "Orbit";
				static ATOM      windowClass  = [ & ]
				{
					auto wndproc = []( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
					{
						LONG_PTR userData = GetWindowLongPtrA( hwnd, GWLP_USERDATA );
						if( userData == 0 )
							return DefWindowProcA( hwnd, msg, wParam, lParam );

						auto w = reinterpret_cast< window* >( userData );
						switch( msg )
						{
							case WM_MOVE:
								w->queue_event( { window_event::Move, { LOWORD( lParam ), HIWORD( lParam ) } } );
								break;

							case WM_SIZE:
								w->queue_event( { window_event::Resize, { LOWORD( lParam ), HIWORD( lParam ) } } );
								break;

							case WM_ACTIVATE:
								if( HIWORD( wParam ) != 0 )
									w->queue_event( { ( LOWORD( wParam ) == WA_INACTIVE ) ? window_event::Suspend : window_event::Restore } );
								break;

							case WM_SETFOCUS:
								w->queue_event( { window_event::Focus } );
								break;

							case WM_KILLFOCUS:
								w->queue_event( { window_event::Defocus } );
								break;

							case WM_CLOSE:
								w->close();
								w->queue_event( { window_event::Close } );
								break;

							default:
								break;
						}

						return DefWindowProcA( hwnd, msg, wParam, lParam );
					};

					WNDCLASSEXA classDesc{ };
					classDesc.cbSize        = sizeof( WNDCLASSEXA );
					classDesc.style         = CS_VREDRAW | CS_HREDRAW;
					classDesc.lpfnWndProc   = wndproc;
					classDesc.hInstance     = GetModuleHandleA( nullptr );
					classDesc.hbrBackground = reinterpret_cast< HBRUSH >( COLOR_WINDOW );
					classDesc.lpszClassName = ClassName;

					/* Extract and copy icon from application. */
					char path[ MAX_PATH + 1 ];
					GetModuleFileNameA( nullptr, path, sizeof( path ) );
					classDesc.hIcon = ExtractIconA( classDesc.hInstance, path, 0 );

					return RegisterClassExA( &classDesc );
				}();

				/* Create window */
				impl->hwnd = CreateWindowExA( WS_EX_OVERLAPPEDWINDOW, ClassName, NULL, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandleA( NULL ), NULL );

				/* Set user data */
				SetWindowLongPtrA( impl->hwnd, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( this ) );

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			case window_impl_type::X11:
			{
				/* Open display */
				auto impl     = std::addressof( m_impl.emplace< __window_impl_x11 >() );
				impl->display = XOpenDisplay( nullptr );

				/* Create window */
				const int               screen     = DefaultScreen( impl->display );
				Window                  rootWindow = XRootWindow( impl->display, screen );
				int                     depth      = DefaultDepth( impl->display, screen );
				Visual*                 visual     = DefaultVisual( impl->display, screen );
				constexpr unsigned long kValueMask = ( CWBackPixel | CWEventMask );
				XSetWindowAttributes    attribs    = { };
				attribs.event_mask                 = ( FocusChangeMask | ResizeRedirectMask | StructureNotifyMask );
				impl->window = XCreateWindow( impl->display, rootWindow, 0, 0, width, height, 0, depth, InputOutput, visual, kValueMask, &attribs );

				/* Allow us to capture the window close event */
				Atom closeAtom = XInternAtom( impl->display, "WM_DELETE_WINDOW", True );
				XSetWMProtocols( impl->display, impl->window, &closeAtom, 1 );

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			case window_impl_type::Wayland:
			{
				m_impl.emplace< __window_impl_wayland >()

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			case window_impl_type::Cocoa:
			{
				auto               impl      = std::addressof( m_impl.emplace< __window_impl_cocoa >() );
				NSRect             frame     = NSMakeRect( 0.0f, 0.0f, width, height );
				NSWindowStyleMask  styleMask = ( NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable );
				NSBackingStoreType backing   = ( NSBackingStoreBuffered );

				/* Create window */
				impl->nsWindow = [ NSWindow alloc ];
				[ ( NSWindow* )impl->nsWindow initWithContentRect:frame styleMask:styleMask backing:backing defer:NO] ;

				/* Create window delegate */
				impl->delegate = [ WindowDelegate alloc ];
				[ ( NSWindow* )impl->nsWindow setDelegate:( ORBCocoaWindowDelegate* )impl->delegate ];
				[ ( ORBCocoaWindowDelegate* )impl->delegate setWindowPtr:this ];
				[ ( ORBCocoaWindowDelegate* )impl->delegate setImpl:&m_impl ];

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			case window_impl_type::Android:
			{
				auto appCmd = []( android_app* state, int cmd )
				{
					window* w = static_cast< window* >( state->userData );
					switch( cmd )
					{
						case APP_CMD_INIT_WINDOW:
						{
							window_event resizeEvent{ window_event::Resize };
							resizeEvent.data.resize.w = static_cast< uint32_t >( ANativeWindow_getWidth( state->window ) );
							resizeEvent.data.resize.h = static_cast< uint32_t >( ANativeWindow_getHeight( state->window ) );

							w->queue_event( resizeEvent );
							w->queue_event( { window_event::Restore } );
							break;
						}

						case APP_CMD_TERM_WINDOW:
							w->queue_event( { window_event::Suspend } );
							break;

						case APP_CMD_GAINED_FOCUS:
						{
							auto impl = reinterpret_cast< __window_impl_android* >( w->get_package_ptr() );
							ASensorEventQueue_enableSensor( impl->sensorEventQueue, impl->accelerometerSensor );
							ASensorEventQueue_setEventRate( impl->sensorEventQueue, impl->accelerometerSensor, ( 1000 * 1000 / 60 ) );
							w->queue_event( { window_event::Focus } );
							break;
						}

						case APP_CMD_LOST_FOCUS:
						{
							auto impl = reinterpret_cast< __window_impl_android* >( w->get_package_ptr() );
							ASensorEventQueue_disableSensor( impl->sensorEventQueue, impl->accelerometerSensor );
							wnd.queue_event( { window_event::Defocus } );
							break;
						}

						case APP_CMD_DESTROY:
							w->close();
							break;

						default:
							break;
					}
				};

				auto onInput = []( android_app * state, AInputEvent * e )
				{
					switch( AInputEvent_getType( e ) )
					{
						default:
							break;
					}

					return 0;
				};

				android_only::app->onInputEvent = onInput;

				auto impl                 = std::addressof( m_impl.emplace< __window_impl_android >() );
				impl->sensorManager       = ASensorManager_getInstance();
				impl->accelerometerSensor = ASensorManager_getDefaultSensor( impl->sensorManager, ASENSOR_TYPE_ACCELEROMETER );
				impl->sensorEventQueue    = ASensorManager_createEventQueue( impl->sensorManager, android_only::app->looper, LOOPER_ID_USER, nullptr, nullptr );

				/* Update until native window is initialized. */
				{
					bool initialized = false;
					android_only::app->userData = &initialized;
					android_only::app->onAppCmd = []( android_app* state, int cmd )
					{
						if( cmd == APP_CMD_INIT_WINDOW )
						{
							auto initialized = static_cast< bool* >( state->userData );
							*initialized = true;
						}
					};

					while( !initialized )
					{
						poll_events();
					}
				}

				android_only::app->userData = this;
				android_only::app->onAppCmd = appCmd;

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			case window_impl_type::UiKit:
			{
				auto impl = std::addressof( m_impl.emplace< __window_impl_uikit >() );

				/* Initialize window */
				impl->uiWindow = [ ORBWindow alloc ];
				[ ( ORBUiKitWindow* )impl->uiWindow initWithFrame:[ [ UIScreen mainScreen ]bounds ] ];
				( ( ORBUiKitWindow* )impl->uiWindow ).backgroundColor = [ UIColor whiteColor ];
				[ ( ORBUiKitWindow* )impl->uiWindow makeKeyAndVisible];
				[ ( ORBUiKitWindow* )impl->uiWindow setWindowPtr:this ];

				/* Create view controller */
				UIViewController* vc = [ UIViewController alloc ];
				[ vc initWithNibName:nil bundle:nil ];
				( ( ORBUiKitWindow* )impl->uiWindow ).rootViewController = vc;

				break;
			}
	#endif
		}
	}

	window::~window()
	{
		switch( m_impl.index() )
		{
			default:
			{
				break;
			}

		#if __ORB_HAS_WINDOW_IMPL_WIN32
			case( window_impl_index_v< __window_impl_win32 > ):
			{
				auto impl = std::get_if< __window_impl_win32 >( &m_impl );
				DestroyWindow( impl->hwnd );
				break;
			}
		#endif

		#if __ORB_HAS_WINDOW_IMPL_X11
			case( window_impl_index_v< __window_impl_x11 > ):
			{
				auto impl = std::get_if< __window_impl_x11 >( &m_impl );
				XDestroyWindow( impl->display, impl->window );
				XCloseDisplay( impl->display );
				break;
			}
		#endif

		#if __ORB_HAS_WINDOW_IMPL_WAYLAND
//			case( window_impl_index_v< __window_impl_wayland > ):
//			{
//				break;
//			}
		#endif

		#if __ORB_HAS_WINDOW_IMPL_COCOA
			case( window_impl_index_v< __window_impl_cocoa > ):
			{
				auto impl = std::get_if< __window_impl_cocoa >( &m_impl );
				[ ( NSWindow* )impl->nsWindow close ];
				[ ( ORBCocoaWindowDelegate* )impl->delegate dealloc ];
				[ ( NSWindow* )impl->nsWindow dealloc ];
				break;
			}
		#endif

		#if __ORB_HAS_WINDOW_IMPL_ANDROID
			case( window_impl_index_v< __window_impl_android > ):
			{
				ASensorManager_destroyEventQueue( impl->sensorManager, impl->sensorEventQueue );
				android_only::app->userData = nullptr;
				android_only::app->onAppCmd = nullptr;
				break;
			}
		#endif

		#if __ORB_HAS_WINDOW_IMPL_UIKIT
			case( window_impl_index_v< __window_impl_uikit > ):
			{
				auto impl = std::get_if< __window_impl_uikit >( &m_impl );
				[ ( ORBUiKitWindow* )impl->uiWindow dealloc ];
				break;
			}
		#endif
		}
	}

	void window::poll_events()
	{
		switch( m_impl.index() )
		{
			default:
			{
				break;
			}

	#if __ORB_HAS_WINDOW_IMPL_WIN32
			case ( window_impl_index_v< __window_impl_win32 > ):
			{
				auto impl = std::get_if< __window_impl_win32 >( &m_impl );

				MSG msg;
				while( PeekMessageA( &msg, impl->hwnd, 0, 0, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessageA( &msg );
				}

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			case ( window_impl_index_v< __window_impl_x11 > ):
			{
				auto impl = std::get_if< __window_impl_x11 >( &m_impl );
				while( XPending( impl->display ) )
				{
					XEvent xevent;
					XNextEvent( impl->display, &xevent );

					switch( xevent.type )
					{
						case FocusIn:
						{
							if( xevent.xfocus.mode != NotifyNormal )
								break;

							window_event e{ };
							e.type = window_event::Focus;
							queue_event( e );
							break;
						}

						case FocusOut:
						{
							if( xevent.xfocus.mode != NotifyNormal )
								break;

							window_event e{ };
							e.type = window_event::Defocus;
							queue_event( e );
							break;
						}

						case ResizeRequest:
						{
							window_event e{ };
							e.type          = window_event::Resize;
							e.data.resize.w = xevent.xresizerequest.width;
							e.data.resize.h = xevent.xresizerequest.height;
							queue_event( e );
							break;
						}

						case ConfigureNotify:
						{
							window_event e{ };
							e.type        = window_event::Move;
							e.data.move.x = xevent.xconfigure.x;
							e.data.move.y = xevent.xconfigure.y;
							queue_event( e );
							break;
						}

						case ClientMessage:
						{
							close();
							break;
						}

						default:
							break;
					}
				}
				break;
			}
	#endif

//	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
//			case ( window_impl_index_v< __window_impl_wayland > ):
//			{
//				break;
//			}
//	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			case ( window_impl_index_v< __window_impl_cocoa > ):
			{
				auto     impl = std::get_if< __window_impl_cocoa >( &m_impl );
				NSEvent* nsEvent;
				while( ( nsEvent = [ ( const NSWindow* )impl->nsWindow nextEventMatchingMask:NSEventMaskAny untilDate:nullptr inMode:NSDefaultRunLoopMode dequeue:YES ] ) != nullptr )
				{
					[ ( const NSWindow* )impl->nsWindow sendEvent:nsEvent ];
				}

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			case ( window_impl_index_v< __window_impl_android > ):
			{
				int                  events;
				android_poll_source* source;
				if( ALooper_pollAll( 0, nullptr, &events, reinterpret_cast< void** >( &source ) ) >= 0 )
				{
					if( source )
						source->process( android_only::app, source );
				}

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			case ( window_impl_index_v< __window_impl_uikit > ):
			{
				break;
			}
	#endif
		}

		send_events();
	}

	void window::set_title( std::string_view title )
	{
		switch( m_impl.index() )
		{
			default:
			{
				( void )title;
				break;
			}

	#if __ORB_HAS_WINDOW_IMPL_WIN32
			case ( window_impl_index_v< __window_impl_win32 > ):
			{
				auto impl = std::get_if< __window_impl_win32 >( &m_impl );
				SetWindowTextA( impl->hwnd, title.data() );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			case ( window_impl_index_v< __window_impl_x11 > ):
			{
				auto impl = std::get_if< __window_impl_x11 >( &m_impl );
				XStoreName( impl->display, impl->window, title.data() );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			case ( window_impl_index_v< __window_impl_wayland > ):
			{
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			case ( window_impl_index_v< __window_impl_cocoa > ):
			{
				auto      impl    = std::get_if< __window_impl_cocoa >( &m_impl );
				NSString* nsTitle = [ NSString stringWithUTF8String:title.data() ];
				[ ( const NSWindow* )impl->nsWindow setTitle:nsTitle ];
				[ nsTitle release ];

				break;
			}
	#endif

//	#if __ORB_HAS_WINDOW_IMPL_ANDROID
//			case ( window_impl_index_v< __window_impl_android > ):
//			{
//				// #TODO: Activity.setTitle
//				break;
//			}
//	#endif

//	#if __ORB_HAS_WINDOW_IMPL_UIKIT
//			case ( window_impl_index_v< __window_impl_uikit > ):
//			{
//				break;
//			}
//	#endif
		}
	}

	void window::set_pos( uint32_t x, uint32_t y )
	{
		switch( m_impl.index() )
		{
			default:
			{
				( void )x;
				( void )y;
				break;
			}

	#if __ORB_HAS_WINDOW_IMPL_WIN32
			case ( window_impl_index_v< __window_impl_win32 > ):
			{
				auto impl = std::get_if< __window_impl_win32 >( &m_impl );
				RECT rect{ };
				GetWindowRect( impl->hwnd, &rect );
				MoveWindow( impl->hwnd, x, y, ( rect.right - rect.left ), ( rect.bottom - rect.top ), FALSE );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			case ( window_impl_index_v< __window_impl_x11 > ):
			{
				auto impl = std::get_if< __window_impl_x11 >( &m_impl );
				XMoveWindow( impl->display, impl->window, x, y );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			case ( window_impl_index_v< __window_impl_wayland > ):
			{
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			case ( window_impl_index_v< __window_impl_cocoa > ):
			{
				auto   impl    = std::get_if< __window_impl_cocoa >( &m_impl );
				NSRect frame   = [ ( const NSWindow* )impl->nsWindow frame ];
				frame.origin.x = x;
				frame.origin.y = y;
				[ ( const NSWindow* )impl->nsWindow setFrame:frame display:YES ];

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			case ( window_impl_index_v< __window_impl_android > ):
			{
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			case ( window_impl_index_v< __window_impl_uikit > ):
			{
				break;
			}
	#endif
		}
	}

	void window::set_size( uint32_t width, uint32_t height )
	{
		switch( m_impl.index() )
		{
			default:
			{
				( void )width;
				( void )height;
				break;
			}

	#if __ORB_HAS_WINDOW_IMPL_WIN32
			case ( window_impl_index_v< __window_impl_win32 > ):
			{
				auto impl = std::get_if< __window_impl_win32 >( &m_impl );
				RECT rect{ };
				GetWindowRect( impl->hwnd, &rect );
				MoveWindow( impl->hwnd, rect.left, rect.top, static_cast< int >( width ), static_cast< int >( height ), FALSE );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			case ( window_impl_index_v< __window_impl_x11 > ):
			{
				auto impl = std::get_if< __window_impl_x11 >( &m_impl );
				XResizeWindow( impl->display, impl->window, width, height );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			case ( window_impl_index_v< __window_impl_wayland > ):
			{
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			case ( window_impl_index_v< __window_impl_cocoa > ):
			{
				auto   impl       = std::get_if< __window_impl_cocoa >( &m_impl );
				NSRect frame      = [ ( const NSWindow* )impl->nsWindow frame ];
				frame.size.width  = width;
				frame.size.height = height;
				[ ( const NSWindow* )impl->nsWindow setFrame:frame display:YES ];

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			case ( window_impl_index_v< __window_impl_android > ):
			{
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			case ( window_impl_index_v< __window_impl_uikit > ):
			{
				break;
			}
	#endif
		}
	}

	void window::show()
	{
		switch( m_impl.index() )
		{
			default:
			{
				break;
			}

	#if __ORB_HAS_WINDOW_IMPL_WIN32
			case ( window_impl_index_v< __window_impl_win32 > ):
			{
				auto impl = std::get_if< __window_impl_win32 >( &m_impl );
				ShowWindow( impl->hwnd, SW_SHOW );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			case ( window_impl_index_v< __window_impl_x11 > ):
			{
				auto impl = std::get_if< __window_impl_x11 >( &m_impl );
				XMapWindow( impl->display, impl->window );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			case ( window_impl_index_v< __window_impl_wayland > ):
			{
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			case ( window_impl_index_v< __window_impl_cocoa > ):
			{
				auto impl = std::get_if< __window_impl_cocoa >( &m_impl );
				[ ( const NSWindow* )impl->nsWindow setIsVisible:YES ];

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			case ( window_impl_index_v< __window_impl_android > ):
			{
				// #TODO: Activity.setVisible
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			case ( window_impl_index_v< __window_impl_uikit > ):
			{
				auto impl = std::get_if< __window_impl_uikit >( &m_impl );
				[ ( ORBUiKitWindow* )impl->uiWindow setHidden:NO ];

				break;
			}
	#endif
		}
	}

	void window::hide()
	{
		switch( m_impl.index() )
		{
			default:
			{
				break;
			}

	#if __ORB_HAS_WINDOW_IMPL_WIN32
			case ( window_impl_index_v< __window_impl_win32 > ):
			{
				auto impl = std::get_if< __window_impl_win32 >( &m_impl );
				ShowWindow( impl->hwnd, SW_HIDE );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			case ( window_impl_index_v< __window_impl_x11 > ):
			{
				auto impl = std::get_if< __window_impl_x11 >( &m_impl );
				XUnmapWindow( impl->display, impl->window );
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			case ( window_impl_index_v< __window_impl_wayland > ):
			{
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			case ( window_impl_index_v< __window_impl_cocoa > ):
			{
				auto impl = std::get_if< __window_impl_cocoa >( &m_impl );
				[ ( const NSWindow* )impl->nsWindow setIsVisible:NO ];

				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			case ( window_impl_index_v< __window_impl_android > ):
			{
				// #TODO: Activity.setVisible
				break;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			case ( window_impl_index_v< __window_impl_uikit > ):
			{
				auto impl = std::get_if< __window_impl_uikit >( &m_impl );
				[ ( ORBUiKitWindow* )impl->uiWindow setHidden:YES ];

				break;
			}
	#endif
		}
	}
}

#if __ORB_HAS_WINDOW_IMPL_COCOA

@implementation ORBCocoaWindowDelegate

-( void )windowWillClose:( NSNotification* )notification
{
	_windowPtr->close();
}

-( void )windowDidMove:( NSNotification* )notification
{
	auto          impl  = std::get_if< __window_impl_cocoa >( _impl );
	const CGPoint point = ( ( const NSWindow* )impl->nsWindow ).frame.origin;

	orb::window_event e{ };
	e.type        = orb::window_event::Move;
	e.data.move.x = static_cast< int >( point.x );
	e.data.move.y = static_cast< int >( point.y );
	_windowPtr->queue_event( e );
}

-( NSSize )windowWillResize:( NSWindow* )sender toSize:( NSSize )frameSize
{
	orb::window_event e{ };
	e.type          = orb::window_event::Resize;
	e.data.resize.w = static_cast< uint32_t >( frameSize.width );
	e.data.resize.h = static_cast< uint32_t >( frameSize.height );
	_windowPtr->queue_event( e );

	return frameSize;
}

-( void )windowDidMiniaturize:( NSNotification* )notification
{
	orb::window_event e{ };
	e.type = orb::window_event::Suspend;
	_windowPtr->queue_event( e );
}

-( void )windowDidDeminiaturize:( NSNotification* )notification
{
	orb::window_event e{ };
	e.type = orb::window_event::Restore;
	_windowPtr->queue_event( e );
}

-( void )windowDidBecomeMain:( NSNotification* )notification
{
	orb::window_event e{ };
	e.type = orb::window_event::Focus;
	_windowPtr->queue_event( e );
}

-( void )windowDidResignMain:( NSNotification* )notification
{
	orb::window_event e{ };
	e.type = orb::window_event::Defocus;
	_windowPtr->queue_event( e );
}

@end

#endif

#if __ORB_HAS_WINDOW_IMPL_UIKIT

@implementation ORBUiKitWindow

-( void )layoutSubviews
{
	[ super layoutSubviews ];

	orb::window_event e{ };
	e.type          = orb::window_event::Resize;
	e.data.resize.w = static_cast< uint32_t >( self.bounds.size.width );
	e.data.resize.h = static_cast< uint32_t >( self.bounds.size.height );
	_windowPtr->queue_event( e );
}

@end

#endif
