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

#if ( __ORB_NUM_WINDOW_IMPLS > 1 )
#  define MAGIC_SWITCH switch( m_implType )
#  define MAGIC_CASE( TYPE ) case TYPE:
#  define MAGIC_BREAK break
#else
#  define MAGIC_SWITCH
#  define MAGIC_CASE( TYPE )
#  define MAGIC_BREAK
#endif

#if __ORB_HAS_WINDOW_IMPL_COCOA
@interface ORBCocoaWindowDelegate : NSObject< NSWindowDelegate >
@property orb::window*              windowPtr;
@property orb::window_impl_storage* storage;
@end
#endif

#if __ORB_HAS_WINDOW_IMPL_UIKIT
@interface ORBUiKitWindow : UIWindow
@property orb::window* windowPtr;
@end
#endif

namespace orb
{
	window::window( uint32_t width, uint32_t height, window_impl_type implType )
		: m_storage{ }
		, m_open( true )
	#if ( __ORB_NUM_WINDOW_IMPLS > 1 )
		, m_implType( implType )
	#endif
	{
		( void )implType;

		MAGIC_SWITCH
		{
	#if __ORB_HAS_WINDOW_IMPL_WIN32
			MAGIC_CASE( window_impl_type::Win32 )
			{
				auto             impl        = &( m_storage.win32 = { } );
				constexpr LPCSTR ClassName   = "Orbit";
				static ATOM      windowClass = [ & ]
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

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			MAGIC_CASE( window_impl_type::X11 )
			{
				/* Open display */
				auto impl     = &( m_storage.x11 = { } );
				impl->display = XOpenDisplay( nullptr );

				/* Create window */
				const int               screen     = DefaultScreen( impl->display );
				Window                  rootWindow = XRootWindow( impl->display, screen );
				int                     depth      = DefaultDepth( impl->display, screen );
				Visual*                 visual     = DefaultVisual( impl->display, screen );
				constexpr unsigned long ValueMask  = ( CWBackPixel | CWEventMask );
				XSetWindowAttributes    attribs    = { };
				attribs.event_mask                 = ( FocusChangeMask | ResizeRedirectMask | StructureNotifyMask );
				impl->window = XCreateWindow( impl->display, rootWindow, 0, 0, width, height, 0, depth, InputOutput, visual, ValueMask, &attribs );

				/* Allow us to capture the window close event */
				Atom closeAtom = XInternAtom( impl->display, "WM_DELETE_WINDOW", True );
				XSetWMProtocols( impl->display, impl->window, &closeAtom, 1 );

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			MAGIC_CASE( window_impl_type::Wayland )
			{
				m_storage.wl = { };

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			MAGIC_CASE( window_impl_type::Cocoa )
			{
				auto               impl     = &( m_storage.cocoa = { } );
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
				[ ( ORBCocoaWindowDelegate* )impl->delegate setStorage:&m_storage ];

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			MAGIC_CASE( window_impl_type::Android )
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

				auto impl                 = &( m_storage.android = { } );
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

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			MAGIC_CASE( window_impl_type::UiKit )
			{
				auto impl = &( m_storage.uikit = { } );

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

				MAGIC_BREAK;
			}
	#endif
		}
	}

	void window::poll_events()
	{
		MAGIC_SWITCH
		{
	#if __ORB_HAS_WINDOW_IMPL_WIN32
			MAGIC_CASE( window_impl_type::Win32 )
			{
				MSG  msg;
				while( PeekMessageA( &msg, m_storage.win32.hwnd, 0, 0, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessageA( &msg );
				}
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			MAGIC_CASE( window_impl_type::X11 )
			{
				while( XPending( m_storage.x11.display ) )
				{
					XEvent xevent;
					XNextEvent( m_storage.x11.display, &xevent );

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
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			MAGIC_CASE( window_impl_type::Wayland )
			{
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			MAGIC_CASE( window_impl_type::Cocoa )
			{
				NSEvent* nsEvent;
				while( ( nsEvent = [ ( const NSWindow* )m_storage.cocoa.nsWindow nextEventMatchingMask:NSEventMaskAny untilDate:nullptr inMode:NSDefaultRunLoopMode dequeue:YES ] ) != nullptr )
				{
					[ ( const NSWindow* )m_storage.cocoa.nsWindow sendEvent:nsEvent ];
				}

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			MAGIC_CASE( window_impl_type::Android )
			{
				int                  events;
				android_poll_source* source;
				if( ALooper_pollAll( 0, nullptr, &events, reinterpret_cast< void** >( &source ) ) >= 0 )
				{
					if( source )
						source->process( android_only::app, source );
				}

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			MAGIC_CASE( window_impl_type::UiKit )
			{
				MAGIC_BREAK;
			}
	#endif
		}

		send_events();
	}

	void window::set_title( std::string_view title )
	{
		MAGIC_SWITCH
		{
	#if __ORB_HAS_WINDOW_IMPL_WIN32
			MAGIC_CASE( window_impl_type::Win32 )
			{
				SetWindowTextA( m_storage.win32.hwnd, title.data() );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			MAGIC_CASE( window_impl_type::X11 )
			{
				XStoreName( m_storage.x11.display, m_storage.x11.window, title.data() );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			MAGIC_CASE( window_impl_type::Wayland )
			{
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			MAGIC_CASE( window_impl_type::Cocoa )
			{
				NSString* nsTitle = [ NSString stringWithUTF8String:title.data() ];
				[ ( const NSWindow* )m_storage.cocoa.nsWindow setTitle:nsTitle ];
				[ nsTitle release ];

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			MAGIC_CASE( window_impl_type::Android )
			{
				// #TODO: Activity.setTitle
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			MAGIC_CASE( window_impl_type::UiKit )
			{
				MAGIC_BREAK;
			}
	#endif
		}
	}

	void window::set_pos( uint32_t x, uint32_t y )
	{
		MAGIC_SWITCH
		{
	#if __ORB_HAS_WINDOW_IMPL_WIN32
			MAGIC_CASE( window_impl_type::Win32 )
			{
				RECT rect{ };
				GetWindowRect( m_storage.win32.hwnd, &rect );
				MoveWindow( m_storage.win32.hwnd, x, y, ( rect.right - rect.left ), ( rect.bottom - rect.top ), FALSE );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			MAGIC_CASE( window_impl_type::X11 )
			{
				XMoveWindow( m_storage.x11.display, m_storage.x11.window, x, y );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			MAGIC_CASE( window_impl_type::Wayland )
			{
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			MAGIC_CASE( window_impl_type::Cocoa )
			{
				NSRect frame   = [ ( const NSWindow* )m_storage.cocoa.nsWindow frame ];
				frame.origin.x = x;
				frame.origin.y = y;
				[ ( const NSWindow* )m_storage.cocoa.nsWindow setFrame:frame display:YES ];

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			MAGIC_CASE( window_impl_type::Android )
			{
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			MAGIC_CASE( window_impl_type::UiKit )
			{
				MAGIC_BREAK;
			}
	#endif
		}
	}

	void window::set_size( uint32_t width, uint32_t height )
	{
		MAGIC_SWITCH
		{
	#if __ORB_HAS_WINDOW_IMPL_WIN32
			MAGIC_CASE( window_impl_type::Win32 )
			{
				RECT rect{ };
				GetWindowRect( m_storage.win32.hwnd, &rect );
				MoveWindow( m_storage.win32.hwnd, rect.left, rect.top, static_cast< int >( width ), static_cast< int >( height ), FALSE );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			MAGIC_CASE( window_impl_type::X11 )
			{
				XResizeWindow( m_storage.x11.display, m_storage.x11.window, width, height );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			MAGIC_CASE( window_impl_type::Wayland )
			{
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			MAGIC_CASE( window_impl_type::Cocoa )
			{
				NSRect frame      = [ ( const NSWindow* )m_storage.cocoa.nsWindow frame ];
				frame.size.width  = width;
				frame.size.height = height;
				[ ( const NSWindow* )m_storage.cocoa.nsWindow setFrame:frame display:YES ];

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			MAGIC_CASE( window_impl_type::Android )
			{
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			MAGIC_CASE( window_impl_type::UiKit )
			{
				MAGIC_BREAK;
			}
	#endif
		}
	}

	void window::show()
	{
		MAGIC_SWITCH
		{
	#if __ORB_HAS_WINDOW_IMPL_WIN32
			MAGIC_CASE( window_impl_type::Win32 )
			{
				ShowWindow( m_storage.win32.hwnd, SW_SHOW );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			MAGIC_CASE( window_impl_type::X11 )
			{
				XMapWindow( m_storage.x11.display, m_storage.x11.window );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			MAGIC_CASE( window_impl_type::Wayland )
			{
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			MAGIC_CASE( window_impl_type::Cocoa )
			{
				[ ( const NSWindow* )m_storage.cocoa.nsWindow setIsVisible:YES ];

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			MAGIC_CASE( window_impl_type::Android )
			{
				// #TODO: Activity.setVisible
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			MAGIC_CASE( window_impl_type::UiKit )
			{
				[ ( ORBUiKitWindow* )m_storage.uikit.uiWindow setHidden:NO ];

				MAGIC_BREAK;
			}
	#endif
		}
	}

	void window::hide()
	{
		MAGIC_SWITCH
		{
	#if __ORB_HAS_WINDOW_IMPL_WIN32
			MAGIC_CASE( window_impl_type::Win32 )
			{
				ShowWindow( m_storage.win32.hwnd, SW_HIDE );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_X11
			MAGIC_CASE( window_impl_type::X11 )
			{
				XUnmapWindow( m_storage.x11.display, m_storage.x11.window );
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
			MAGIC_CASE( window_impl_type::Wayland )
			{
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_COCOA
			MAGIC_CASE( window_impl_type::Cocoa )
			{
				[ ( const NSWindow* )m_storage.cocoa.nsWindow setIsVisible:NO ];

				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_ANDROID
			MAGIC_CASE( window_impl_type::Android )
			{
				// #TODO: Activity.setVisible
				MAGIC_BREAK;
			}
	#endif

	#if __ORB_HAS_WINDOW_IMPL_UIKIT
			MAGIC_CASE( window_impl_type::UiKit )
			{
				[ ( ORBUiKitWindow* )m_storage.uikit.uiWindow setHidden:YES ];

				MAGIC_BREAK;
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
	const CGPoint point = ( ( const NSWindow* )_storage.uikit.nsWindow ).frame.origin;

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
