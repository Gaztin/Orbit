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

#include "RenderContext.h"

#include <array>
#include <cstring>

#include "Orbit/Core/IO/Log.h"
#include "Orbit/Core/Platform/Android/AndroidApp.h"
#include "Orbit/Core/Platform/Windows/ComPtr.h"
#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Core/Widget/Window.h"

#if defined( ORB_OS_ANDROID )
#  include <android/native_window.h>
#  include <android_native_app_glue.h>
#endif

#if _ORB_HAS_WINDOW_API_UIKIT
#  include <GLKit/GLKit.h>
@interface OrbitGLKViewDelegate : UIResponder< GLKViewDelegate >
@end
#endif

ORB_NAMESPACE_BEGIN

static RenderContext* current_context = nullptr;

template< typename T >
constexpr auto window_impl_index_v = unique_index_v< T, WindowImpl >;

template< typename T >
constexpr auto render_context_impl_index_v = unique_index_v< T, RenderContextImpl >;

#if _ORB_HAS_GRAPHICS_API_D3D11
constexpr DXGI_FORMAT kBackBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr DXGI_FORMAT kDepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
#endif
#if _ORB_HAS_GRAPHICS_API_OPENGL
template< typename T >
constexpr auto opengl_render_context_impl_index_v = unique_index_v< T, _RenderContextImplOpenGL::_SubImpl >;
#endif

RenderContext::RenderContext( [[ maybe_unused ]] Window& parent_window, GraphicsAPI api )
	: m_impl                { }
	, m_resize_subscription { }
{
	switch( api )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case GraphicsAPI::OpenGL:
		{
			auto parent_window_impl = parent_window.GetImplPtr();
			auto impl               = std::addressof( m_impl.emplace< _RenderContextImplOpenGL >() );
			switch( parent_window_impl->index() )
			{
				default: break;

			#if _ORB_HAS_WINDOW_API_WIN32
				case( window_impl_index_v< _WindowImplWin32 > ):
				{
					auto sub_impl = std::addressof( impl->sub_impl.emplace< _RenderContextImplOpenGL::_SubImplWin32 >() );
					sub_impl->parent_window_impl = parent_window_impl;

					/* Set pixel format */
					{
						auto window_impl = std::get_if< _WindowImplWin32 >( sub_impl->parent_window_impl );
						sub_impl->hdc    = GetDC( window_impl->hwnd );

						PIXELFORMATDESCRIPTOR desc { };
						desc.nSize      = sizeof( PIXELFORMATDESCRIPTOR );
						desc.nVersion   = 1;
						desc.dwFlags    = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
						desc.iPixelType = PFD_TYPE_RGBA;
						desc.cColorBits = 24;
						desc.cDepthBits = 32;
						desc.iLayerType = PFD_MAIN_PLANE;
						const int format = ChoosePixelFormat( sub_impl->hdc, &desc );

						SetPixelFormat( sub_impl->hdc, format, &desc );
					}

					/* Create dummy context */
					HGLRC dummy_context = wglCreateContext( sub_impl->hdc );
					wglMakeCurrent( sub_impl->hdc, dummy_context );

					/* Create persistent context */
					{
						using wglChoosePixelFormatARB_t = BOOL( WINAPI* )( HDC hdc, const int* i_attribs, const FLOAT* f_attribs, UINT max_formats, int* formats, UINT* num_formats );
						auto  wglChoosePixelFormatARB   = reinterpret_cast< wglChoosePixelFormatARB_t >( wglGetProcAddress( "wglChoosePixelFormatARB" ) );

						using wglCreateContextAttribsARB_t = HGLRC( WINAPI* )( HDC hdc, HGLRC shareContext, const int* attribs );
						auto  wglCreateContextAttribsARB   = reinterpret_cast< wglCreateContextAttribsARB_t >( wglGetProcAddress( "wglCreateContextAttribsARB" ) );

						if( wglChoosePixelFormatARB && wglCreateContextAttribsARB )
						{
							const int format_attributes[]
							{
								0x2010, 1,      // WGL_SUPPORT_OPENGL_ARB
								0x2001, 1,      // WGL_DRAW_TO_WINDOW_ARB
								0x2002, 1,      // WGL_DRAW_TO_BITMAP_ARB
								0x2011, 1,      // WGL_DOUBLE_BUFFER_ARB
								0x2006, 1,      // WGL_SWAP_LAYER_BUFFERS_ARB
								0x2014, 24,     // WGL_COLOR_BITS_ARB
								0x2015, 8,      // WGL_RED_BITS_ARB
								0x2017, 8,      // WGL_GREEN_BITS_ARB
								0x2019, 8,      // WGL_BLUE_BITS_ARB
								0x201B, 0,      // WGL_ALPHA_BITS_ARB
								0x2022, 32,     // WGL_DEPTH_BITS_ARB
								0x2023, 8,      // WGL_STENCIL_BITS_ARB
								0x2003, 0x2027, // WGL_ACCELERATION_ARB = WGL_FULL_ACCELERATION_ARB
								0x2013, 0x202B, // WGL_PIXEL_TYPE_ARB = WGL_TYPE_RGBA_ARB
								0
							};

							int  pixel_formats = 0;
							UINT pixel_format_count = 0;
							wglChoosePixelFormatARB( sub_impl->hdc, format_attributes, nullptr, 1, &pixel_formats, &pixel_format_count );

							const int contextAttributes[]
							{
								0x2091, 4,          // WGL_CONTEXT_MAJOR_VERSION_ARB = 4
								0x2092, 0,          // WGL_CONTEXT_MINOR_VERSION_ARB = 0
								0x9126, 0x00000001, // WGL_CONTEXT_PROFILE_MASK_ARB  = WGL_CONTEXT_CORE_PROFILE_BIT_ARB
								0
							};

							sub_impl->hglrc = wglCreateContextAttribsARB( sub_impl->hdc, nullptr, contextAttributes );
						}
						else
						{
							sub_impl->hglrc = wglCreateContext( sub_impl->hdc );
						}
					}

					/* Destroy dummy context */
					wglDeleteContext( dummy_context );

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_X11
				case( window_impl_index_v< _WindowImplX11 > ):
				{
					auto sub_impl    = std::addressof( impl->sub_impl.emplace< _RenderContextImplOpenGL::_SubImplX11 >() );
					auto window_impl = std::get_if< _WindowImplX11 >( parent_window_impl );

					sub_impl->parent_window_impl = parentWindowImpl;
					sub_impl->gc                 = XCreateGC( window_impl->display, window_impl->window, 0, nullptr );
					sub_impl->glxContext         = [ & ]
					{
						/* Create render context */
						{
							int screen = DefaultScreen( window_impl->display );
							int attribs[]
							{
								GLX_X_RENDERABLE,  True,
								GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
								GLX_RENDER_TYPE,   GLX_RGBA_BIT,
								GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
								GLX_RED_SIZE,      8,
								GLX_GREEN_SIZE,    8,
								GLX_BLUE_SIZE,     8,
								GLX_ALPHA_SIZE,    8,
								GLX_DEPTH_SIZE,    24,
								GLX_STENCIL_SIZE,  8,
								GLX_DOUBLEBUFFER,  True,
								None
							};

							do
							{
								int major, minor;
								if( !glXQueryVersion( window_impl->display, &major, &minor ) )
									break;
								if( ( major < 1 ) || ( major == 1 && minor < 3 ) )
									break;

								using glXCreateContextAttribsARB_t = GLXContext( * )( Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list );
								glXCreateContextAttribsARB_t glXCreateContextAttribsARB = nullptr;
								glXCreateContextAttribsARB = reinterpret_cast< glXCreateContextAttribsARB_t >( glXGetProcAddressARB( reinterpret_cast< const GLubyte* >( "glXCreateContextAttribsARB" ) ) );
								if( !glXCreateContextAttribsARB )
									break;

								int fb_config_count = 0;
								GLXFBConfig* fb_configs = glXChooseFBConfig( window_impl->display, screen, attribs, &fb_config_count );
								if( !fb_configs )
									break;
								if( fb_config_count == 0 )
									break;

								// Choose the best config
								int best_fb_config_idx = 0;
								int best_sample_count = 0;
								for( int i = 0; i < fb_config_count; ++i )
								{
									XVisualInfo* vi = glXGetVisualFromFBConfig( window_impl->display, fb_configs[ i ] );
									if( vi )
									{
										int samples = 0;
										int sample_count = 0;

										glXGetFBConfigAttrib( window_impl->display, fb_configs[ i ], GLX_SAMPLE_BUFFERS, &samples );
										glXGetFBConfigAttrib( window_impl->display, fb_configs[ i ], GLX_SAMPLES, &sample_count );

										if( samples && sample_count > best_sample_count )
										{
											best_fb_config_idx = i;
											best_sample_count  = sampleCount;
										}
									}
									XFree( vi );
								}

								GLXFBConfig best_fb_config = fb_configs[ best_fb_config_idx ];

								XFree( fb_configs );

								int context_attribs[]
								{
									GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
									GLX_CONTEXT_MINOR_VERSION_ARB, 0,
									GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
									None
								};

								return glXCreateContextAttribsARB( window_impl->display, best_fb_config, 0, True, context_attribs );

							} while( false );

							// If all else fails, use legacy method
							XVisualInfo* visual_info = glXChooseVisual( window_impl->display, screen, attribs );
							return glXCreateContext( window_impl->display, visual_info, nullptr, true );
						}
					}();

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_WAYLAND
				case( window_impl_index_v< __window_impl_wayland > ):
				{
					impl->sub_impl.emplace< _RenderContextImplOpenGL::_SubImplWayland >();
					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_COCOA
				case( window_impl_index_v< _WindowImplCocoa > ):
				{
					auto sub_impl    = std::addressof( impl->sub_impl.emplace< _RenderContextImplOpenGL::_SubImplCocoa >() );
					auto window_impl = std::get_if< _WindowImplCocoa >( parent_window_impl );

					const NSOpenGLPixelFormatAttribute attribs[]
					{
						NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
						NSOpenGLPFADoubleBuffer,  true,
						NSOpenGLPFAColorSize,     24,
						NSOpenGLPFADepthSize,     24,
						0
					};

					NSWindow*            ns_indow     = ( NSWindow* )window_impl->ns_window;
					NSOpenGLPixelFormat* pixel_format = [ NSOpenGLPixelFormat alloc ];
					[ pixel_format initWithAttributes:attribs ];

					sub_impl->view = [ NSOpenGLView alloc ];
					[ ( NSOpenGLView* )sub_impl->view initWithFrame:ns_window.contentView.frame pixelFormat:pixel_format ];
					[ ( NSOpenGLView* )sub_impl->view prepareOpenGL ];
					[ ns_window.contentView addSubview:( NSOpenGLView* )sub_impl->view ];

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_ANDROID
				case( window_impl_index_v< _WindowImplAndroid > ):
				{
					auto sub_impl = std::addressof( impl->sub_impl.emplace< _RenderContextImplOpenGL::_SubImplAndroid >() );

					/* Initialize display */
					{
						sub_impl->display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
						eglInitialize( sub_impl->display, nullptr, nullptr );
					}

					/* Choose config */
					sub_impl->config = EGL_NO_CONFIG_KHR;
					do
					{
						EGLint config_count = 0;
						if( !eglGetConfigs( sub_impl->display, nullptr, 0, &config_count ) )
							break;

						std::vector< EGLConfig > configs( static_cast< size_t >( config_count ) );
						if( !eglGetConfigs( sub_impl->display, configs.data(), configs.size(), &config_count ) )
							break;

						const EGLint required_conformant   = EGL_OPENGL_ES3_BIT_KHR;
						const EGLint required_surface_type = ( EGL_WINDOW_BIT | EGL_PBUFFER_BIT );

						EGLint best_red_size    = -1;
						EGLint best_green_size  = -1;
						EGLint best_blue_size   = -1;
						EGLint best_alpha_size  = -1;
						EGLint best_buffer_size = -1;
						EGLint best_depth_size  = -1;

						for( const EGLConfig& config : configs )
						{
							EGLint conformant = 0;
							eglGetConfigAttrib( sub_impl->display, config, EGL_CONFORMANT, &conformant );
							if( ( conformant & required_conformant ) == 0 )
								continue;

							EGLint surface_type = 0;
							eglGetConfigAttrib( sub_impl->display, config, EGL_SURFACE_TYPE, &surface_type );
							if( ( surface_type & required_surface_type ) == 0 )
								continue;

							EGLint red_size = 0;
							eglGetConfigAttrib( sub_impl->display, config, EGL_RED_SIZE, &red_size );
							if( red_size < best_red_size )
								continue;

							EGLint green_size = 0;
							eglGetConfigAttrib( sub_impl->display, config, EGL_RED_SIZE, &green_size );
							if( green_size < best_green_size )
								continue;

							EGLint blue_size = 0;
							eglGetConfigAttrib( sub_impl->display, config, EGL_RED_SIZE, &blue_size );
							if( blue_size < best_blue_size )
								continue;

							EGLint alpha_size = 0;
							eglGetConfigAttrib( sub_impl->display, config, EGL_RED_SIZE, &alpha_size );
							if( alpha_size < best_alpha_size )
								continue;

							EGLint buffer_size = 0;
							eglGetConfigAttrib( sub_impl->display, config, EGL_BUFFER_SIZE, &buffer_size );
							if( buffer_size < best_buffer_size )
								continue;

							EGLint depth_size = 0;
							eglGetConfigAttrib( sub_impl->display, config, EGL_DEPTH_SIZE, &depth_size );
							if( depth_size < best_depth_size )
								continue;

							best_red_size    = red_size;
							best_green_size  = green_size;
							best_buffer_size = blue_size;
							best_alpha_size  = alpha_size;
							best_buffer_size = buffer_size;
							best_depth_size  = depth_size;
							sub_impl->config = config;
						}

						EGLint visual_id = 0;
						eglGetConfigAttrib( sub_impl->display, sub_impl->config, EGL_NATIVE_VISUAL_ID, &visual_id );
						ANativeWindow_setBuffersGeometry( AndroidOnly::app->window, 0, 0, visual_id );

					} while( false );

					/* Create window surface */
					sub_impl->surface = eglCreateWindowSurface( sub_impl->display, sub_impl->config, AndroidOnly::app->window, nullptr );

					/* Create context */
					{
						const EGLint attribs[]
						{
							EGL_CONTEXT_CLIENT_VERSION, 3,
							EGL_NONE,
						};

						sub_impl->context = eglCreateContext( sub_impl->display, sub_impl->config, EGL_NO_CONTEXT, attribs );
					}

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_UIKIT
				case( window_impl_index_v< _WindowImplUIKit > ):
				{
					auto sub_impl    = std::addressof( impl->sub_impl.emplace< _RenderContextImplOpenGL::_SubImplUIKit >() );
					auto window_impl = std::get_if< _WindowImplUIKit >( parent_window_impl );

					UIWindow*             ui_window = ( UIWindow* )window_impl->ui_window;
					OrbitGLKViewDelegate* delegate = [ OrbitGLKViewDelegate alloc ];
					[ delegate init ];

					sub_impl->context = [ EAGLContext alloc ];
					[ ( EAGLContext* )sub_impl->context initWithAPI:kEAGLRenderingAPIOpenGLES3 ];

					sub_impl->view = [ GLKView alloc ];
					[ ( GLKView* )sub_impl->view initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
					( ( GLKView* )sub_impl->view ).context               = ( EAGLContext* )sub_impl->context;
					( ( GLKView* )sub_impl->view ).delegate              = delegate;
					( ( GLKView* )sub_impl->view ).enableSetNeedsDisplay = NO;
					[ ui_window addSubview:( GLKView* )sub_impl->view ];

					break;
				}
			#endif
			}

			/* Load functions */
			MakeCurrent();
			impl->functions.emplace();

			glEnable( GL_CULL_FACE );
			glEnable( GL_DEPTH_TEST );
			glCullFace( GL_BACK );

			auto opengl_version = reinterpret_cast< const char* >( glGetString( GL_VERSION ) );
			auto check_digit    = [ & ]( Version* out )
			{
				/* String begins with version number */
				if( isdigit( opengl_version[ 0 ] ) )
				{
					uint32_t v[ 2 ]{ };
					sscanf( opengl_version, "%u.%u", &v[ 0 ], &v[ 1 ] );
					out->major = static_cast< uint8_t >( v[ 0 ] );
					out->minor = static_cast< uint8_t >( v[ 1 ] );

					return true;
				}

				return false;
			};
			auto parse_version = [ & ]
			{
				Version v;
				if( check_digit( &v ) )
					return v;

				/* OpenGL ... */
				if( std::strncmp( opengl_version, "OpenGL", 6 ) != 0 )
					return Version( 0 );
				if( check_digit( &v ) )
					return v;

				/* OpenGL ES ... */
				opengl_version += 7;
				if( std::strncmp( opengl_version, "ES", 2 ) != 0 )
					return Version( 0 );
				impl->embedded = true;
				opengl_version += 3;
				if( check_digit( &v ) )
					return v;

				/* OpenGL ES-CM ... */
				opengl_version -= 1;
				if( std::strncmp( opengl_version, "-CM", 3 ) != 0 )
					return Version( 0 );
				opengl_version += 4;
				if( check_digit( &v ) )
					return v;

				return Version( 0 );
			};

			impl->opengl_version = parse_version();

			LogInfo( Format( "OpenGL version: %s %d.%d", impl->embedded ? "ES" : "", impl->opengl_version.major, impl->opengl_version.minor ) );

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case GraphicsAPI::D3D11:
		{
			auto impl        = std::addressof( m_impl.emplace< _RenderContextImplD3D11 >() );
			auto window_impl = std::get_if< _WindowImplWin32 >( parent_window.GetImplPtr() );

			/* Find the monitor refresh rate */
			DXGI_RATIONAL refreshRate = { 60000, 1000 };
			{
				ComPtr< IDXGIFactory > factory;
				ComPtr< IDXGIAdapter > adapter;
				ComPtr< IDXGIOutput >  output;
				{ /* Get the primary adapter output. */
					IDXGIObject* tmp;
					CreateDXGIFactory( __uuidof( IDXGIFactory ), reinterpret_cast< void** >( &tmp ) );
					factory.reset( static_cast< IDXGIFactory* >( tmp ) );
					factory->EnumAdapters( 0, reinterpret_cast< IDXGIAdapter** >( &tmp ) );
					adapter.reset( static_cast< IDXGIAdapter* >( tmp ) );
					adapter->EnumOutputs( 0, reinterpret_cast< IDXGIOutput** >( &tmp ) );
					output.reset( static_cast< IDXGIOutput* >( tmp ) );
				}

				UINT monitor_width;
				UINT monitor_height;
				{ /* Get monitor resolution. */
					HMONITOR    monitor = MonitorFromWindow( window_impl->hwnd, MONITOR_DEFAULTTONEAREST );
					MONITORINFO monitor_info;
					monitor_info.cbSize = sizeof( MONITORINFO );
					GetMonitorInfoA( monitor, &monitor_info );

					monitor_width  = ( monitor_info.rcMonitor.right - monitor_info.rcMonitor.left );
					monitor_height = ( monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top );
				}

				std::vector< DXGI_MODE_DESC > display_modes;
				{ /* Get display modes. */
					UINT num_display_modes;
					output->GetDisplayModeList( kBackBufferFormat, DXGI_ENUM_MODES_INTERLACED, &num_display_modes, nullptr );
					display_modes.resize( num_display_modes );
					output->GetDisplayModeList( kBackBufferFormat, DXGI_ENUM_MODES_INTERLACED, &num_display_modes, &display_modes[ 0 ] );
				}

				/* Finally, iterate all the display modes to find one matching the monitor's resolution. */
				for( DXGI_MODE_DESC& mode : display_modes )
				{
					if( mode.Width == monitor_width &&
						mode.Height == monitor_height )
					{
						refreshRate = mode.RefreshRate;

						break;
					}
				}
			}

			/* Create the swap chain */
			{
				constexpr UINT kDeviceFlags = 0
				#if !defined( NDEBUG )
					| D3D11_CREATE_DEVICE_DEBUG
				#endif
					;
				constexpr std::array kFeatureLevels
				{
					D3D_FEATURE_LEVEL_11_1,
					D3D_FEATURE_LEVEL_11_0,
					D3D_FEATURE_LEVEL_10_1,
					D3D_FEATURE_LEVEL_10_0,
					D3D_FEATURE_LEVEL_9_3,
					D3D_FEATURE_LEVEL_9_2,
					D3D_FEATURE_LEVEL_9_1,
				};

				RECT window_rect;
				GetWindowRect( window_impl->hwnd, &window_rect );

				DXGI_SWAP_CHAIN_DESC desc { };
				desc.BufferDesc.Width       = ( window_rect.right - window_rect.left );
				desc.BufferDesc.Height      = ( window_rect.bottom - window_rect.top );
				desc.BufferDesc.RefreshRate = refreshRate;
				desc.BufferDesc.Format      = kBackBufferFormat;
				desc.SampleDesc.Count       = 1;
				desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				desc.BufferCount            = 1;
				desc.OutputWindow           = window_impl->hwnd;
				desc.Windowed               = true;
				desc.SwapEffect             = DXGI_SWAP_EFFECT_DISCARD;

				IDXGISwapChain* swap_chain;
				D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, kDeviceFlags, kFeatureLevels.data(), static_cast< UINT >( kFeatureLevels.size() ), D3D11_SDK_VERSION, &desc, &swap_chain, NULL, NULL, NULL );
				impl->swap_chain.reset( swap_chain );
			}

			/* Get the device */
			{
				void* device;
				impl->swap_chain->GetDevice( __uuidof( ID3D11Device ), &device );
				impl->device.reset( static_cast< ID3D11Device* >( device ) );
			}

			/* Get the device context */
			{
				ID3D11DeviceContext* deviceContext;
				impl->device->GetImmediateContext( &deviceContext );
				impl->device_context.reset( deviceContext );
			}

			/* Create the render target */
			{
				ID3D11Texture2D* back_buffer;
				impl->swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &back_buffer ) );
				ID3D11RenderTargetView* renderTargetView;
				impl->device->CreateRenderTargetView( back_buffer, NULL, &renderTargetView );
				impl->render_target_view.reset( renderTargetView );
				back_buffer->Release();
			}

			/* Create the depth stencil */
			{
				RECT rect { };
				GetWindowRect( window_impl->hwnd, &rect );

				D3D11_TEXTURE2D_DESC buffer_desc { };
				buffer_desc.Width            = ( rect.right - rect.left );
				buffer_desc.Height           = ( rect.bottom - rect.top );
				buffer_desc.MipLevels        = 1;
				buffer_desc.ArraySize        = 1;
				buffer_desc.Format           = kDepthBufferFormat;
				buffer_desc.SampleDesc.Count = 1;
				buffer_desc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

				ID3D11Texture2D* depth_stencil_buffer;
				impl->device->CreateTexture2D( &buffer_desc, NULL, &depth_stencil_buffer );
				impl->depth_stencil_buffer.reset( depth_stencil_buffer );

				D3D11_DEPTH_STENCIL_DESC state_desc { };
				state_desc.DepthEnable                  = true;
				state_desc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
				state_desc.DepthFunc                    = D3D11_COMPARISON_LESS;
				state_desc.StencilEnable                = true;
				state_desc.StencilReadMask              = 0xff;
				state_desc.StencilWriteMask             = 0xff;
				state_desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
				state_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
				state_desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
				state_desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
				state_desc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
				state_desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_DECR;
				state_desc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
				state_desc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

				ID3D11DepthStencilState* depth_stencil_state;
				impl->device->CreateDepthStencilState( &state_desc, &depth_stencil_state );
				impl->device_context->OMSetDepthStencilState( depth_stencil_state, 1 );
				impl->depth_stencil_state.reset( depth_stencil_state );
					
				D3D11_DEPTH_STENCIL_VIEW_DESC view_desc { };
				view_desc.Format        = kDepthBufferFormat;
				view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

				ID3D11DepthStencilView* depth_stencil_view;
				impl->device->CreateDepthStencilView( depth_stencil_buffer, &view_desc, &depth_stencil_view );

				ID3D11RenderTargetView* render_target_views[] = { impl->render_target_view.get() };
				impl->device_context->OMSetRenderTargets( 1, render_target_views, depth_stencil_view );
				impl->depth_stencil_view.reset( depth_stencil_view );
			}

			/* Create rasterizer */
			{
				D3D11_RASTERIZER_DESC desc { };
				desc.CullMode              = D3D11_CULL_BACK;
				desc.DepthClipEnable       = true;
				desc.FillMode              = D3D11_FILL_SOLID;
				desc.FrontCounterClockwise = false;

				ID3D11RasterizerState* rasterizer_state;
				impl->device->CreateRasterizerState( &desc, &rasterizer_state );
				impl->device_context->RSSetState( rasterizer_state );
				impl->rasterizer_state.reset( rasterizer_state );
			}

			/* Set default topology */
			impl->device_context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			break;
		}
	#endif
	}

	// Resize context when window is updated
	m_resize_subscription = parent_window.subscribe( [ this ]( const WindowEvent& e )
		{
			if( e.type == WindowEvent::Resize )
				Resize( e.data.resize.w, e.data.resize.h );
		}
	);

	if( current_context == nullptr )
	{
		MakeCurrent();
	}
}

RenderContext::~RenderContext()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( render_context_impl_index_v< _RenderContextImplOpenGL > ):
		{
			auto impl = std::get_if< _RenderContextImplOpenGL >( &m_impl );

			switch( impl->sub_impl.index() )
			{
				default: break;

			#if _ORB_HAS_WINDOW_API_WIN32
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplWin32 > ):
				{
					auto sub_impl    = std::get_if< _RenderContextImplOpenGL::_SubImplWin32 >( &impl->sub_impl );
					auto window_impl = std::get_if< _WindowImplWin32 >( sub_impl->parent_window_impl );

					if( current_context == this )
						wglMakeCurrent( NULL, NULL );

					wglDeleteContext( sub_impl->hglrc );
					ReleaseDC( window_impl->hwnd, sub_impl->hdc );

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_X11
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplX11 > ):
				{
					auto sub_impl    = std::get_if< _RenderContextImplOpenGL::_SubImplX11 >( &impl->impl );
					auto window_impl = std::get_if< _WindowImplX11 >( sub_impl->parent_window_impl );

					if( current_context == this )
						glXMakeCurrent( sub_impl->display, None, nullptr );

					glXDestroyContext( sub_impl->display, sub_impl->context );
					XFreeGC( sub_impl->display, sub_impl->gc );

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_WAYLAND
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplWayland > ):
				{
					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_COCOA
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplCocoa > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplCocoa >( &impl->impl );

					if( current_context == this )
						[ NSOpenGLContext clearCurrentContext ];

					[ ( const NSOpenGLView* )sub_impl->view removeFromSuperview ];
					[ ( const NSOpenGLView* )sub_impl->view dealloc ];

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_ANDROID
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplAndroid > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplAndroid >( &impl->impl );

					if( current_context == this )
						eglMakeCurrent( sub_impl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );

					eglDestroyContext( sub_impl->display, sub_impl->context );
					eglDestroySurface( sub_impl->display, sub_impl->surface );
					eglTerminate( sub_impl->display );

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_UIKIT
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplUIKit > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplUIKit >( &impl->impl );

					if( current_context == this )
						[ EAGLContext setCurrentContext:nullptr ];

					[ ( GLKView* )sub_impl->view dealloc ];
					[ ( EAGLContext* )sub_impl->context dealloc ];

					break;
				}
			#endif
			}

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( render_context_impl_index_v< _RenderContextImplD3D11 > ):
		{
			break;
		}
	#endif
	}

	if( current_context == this )
	{
		current_context = nullptr;
	}
}

bool RenderContext::MakeCurrent()
{
#if _ORB_HAS_GRAPHICS_API_OPENGL
	if( m_impl.index() == render_context_impl_index_v< _RenderContextImplOpenGL > )
	{
		auto impl = std::get_if< _RenderContextImplOpenGL >( &m_impl );

		switch( impl->sub_impl.index() )
		{
			default: break;

		#if _ORB_HAS_WINDOW_API_WIN32
			case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplWin32 > ):
			{
				auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplWin32 >( &impl->sub_impl );

				if( !wglMakeCurrent( sub_impl->hdc, sub_impl->hglrc ) )
					return false;

				break;
			}
		#endif

		#if _ORB_HAS_WINDOW_API_X11
			case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplX11 > ):
			{
				auto sub_impl    = std::get_if< _RenderContextImplOpenGL::_SubImplX11 >( &impl->impl );
				auto window_impl = std::get_if< _WindowImplX11 >( sub_impl->parent_window_impl );

				if( !glXMakeCurrent( window_impl->display, window_impl->window, sub_impl->context ) )
					return false;

				break;
			}
		#endif

		#if _ORB_HAS_WINDOW_API_WAYLAND
			case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplWayland > ):
			{
				break;
			}
		#endif

		#if _ORB_HAS_WINDOW_API_COCOA
			case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplCocoa > ):
			{
				auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplCocoa >( &impl->impl );
				[ [ ( const NSOpenGLView* )sub_impl->view openGLContext ] makeCurrentContext ];

				break;
			}
		#endif

		#if _ORB_HAS_WINDOW_API_ANDROID
			case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplAndroid > ):
			{
				auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplAndroid >( &impl->impl );
				eglMakeCurrent( sub_impl->display, sub_impl->surface, sub_impl->surface, sub_impl->context );
				break;
			}
		#endif

		#if _ORB_HAS_WINDOW_API_UIKIT
			case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplUIKit > ):
			{
				auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplUIKit >( &impl->impl );
				[ EAGLContext setCurrentContext:( EAGLContext* )sub_impl->context ];
				break;
			}
		#endif
		}
	}
#endif

	current_context = this;

	return true;
}

void RenderContext::Resize( uint32_t width, uint32_t height )
{
	RenderContext* prev_current_context = current_context;
	MakeCurrent();

	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( render_context_impl_index_v< _RenderContextImplOpenGL > ):
		{
		#if _ORB_HAS_WINDOW_API_ANDROID || _ORB_HAS_WINDOW_API_UIKIT

			auto impl = std::get_if< _RenderContextImplOpenGL >( &m_impl );

			switch( impl->sub_impl.index() )
			{
			#if _ORB_HAS_WINDOW_API_ANDROID
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplAndroid > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplAndroid >( &impl->impl );
					eglMakeCurrent( sub_impl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
					if( sub_impl->surface != EGL_NO_SURFACE )
						eglDestroySurface( sub_impl->display, sub_impl->surface );
					sub_impl->surface = eglCreateWindowSurface( sub_impl->display, sub_impl->eglConfig, AndroidOnly::app->window, nullptr );

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_UIKIT
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplUIKit > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplUIKit >( &impl->impl );
					( ( GLKView* )sub_impl->view ).layer.frame = CGRectMake( 0.f, 0.f, width, height );
					break;
				}
			#endif
			}

		#endif

			glViewport( 0, 0, width, height );

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( render_context_impl_index_v< _RenderContextImplD3D11 > ):
		{
			auto impl = std::get_if< _RenderContextImplD3D11 >( &m_impl );

			impl->device_context->OMSetRenderTargets( 0, nullptr, nullptr );
			impl->device_context->ClearState();
			impl->device_context->Flush();

			D3D11_TEXTURE2D_DESC depth_stencil_buffer_desc { };
			impl->depth_stencil_buffer->GetDesc( &depth_stencil_buffer_desc );

			D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc { };
			impl->depth_stencil_view->GetDesc( &depth_stencil_view_desc );

			impl->render_target_view.reset();
			impl->depth_stencil_buffer.reset();
			impl->depth_stencil_view.reset();

			DXGI_SWAP_CHAIN_DESC swap_chain_desc { };
			impl->swap_chain->GetDesc( &swap_chain_desc );
			swap_chain_desc.BufferDesc.Width  = width;
			swap_chain_desc.BufferDesc.Height = height;
			impl->swap_chain->ResizeBuffers( 1, swap_chain_desc.BufferDesc.Width, swap_chain_desc.BufferDesc.Height, swap_chain_desc.BufferDesc.Format, swap_chain_desc.Flags );

			/* Recreate render target */
			{
				ID3D11Texture2D* back_buffer;
				impl->swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &back_buffer ) );
				ID3D11RenderTargetView* renderTargetView;
				impl->device->CreateRenderTargetView( back_buffer, nullptr, &renderTargetView );
				impl->render_target_view.reset( renderTargetView );
				back_buffer->Release();
			}

			/* Recreate depth stencil */
			{
				depth_stencil_buffer_desc.Width  = width;
				depth_stencil_buffer_desc.Height = height;

				ID3D11Texture2D* depth_stencil_buffer;
				impl->device->CreateTexture2D( &depth_stencil_buffer_desc, NULL, &depth_stencil_buffer );
				impl->depth_stencil_buffer.reset( depth_stencil_buffer );

				ID3D11DepthStencilView* depth_stencil_view;
				impl->device->CreateDepthStencilView( depth_stencil_buffer, &depth_stencil_view_desc, &depth_stencil_view );
				impl->depth_stencil_view.reset( depth_stencil_view );
			}

			ID3D11RenderTargetView* render_target_views[] = { impl->render_target_view.get() };
			impl->device_context->OMSetRenderTargets( 1, render_target_views, impl->depth_stencil_view.get() );
			impl->device_context->OMSetDepthStencilState( impl->depth_stencil_state.get(), 0 );
			impl->device_context->RSSetState( impl->rasterizer_state.get() );
			impl->device_context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			D3D11_VIEWPORT viewport { };
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width    = static_cast< FLOAT >( width );
			viewport.Height   = static_cast< FLOAT >( height );
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			impl->device_context->RSSetViewports( 1, &viewport );

			RECT scissor { };
			scissor.left   = 0;
			scissor.right  = width;
			scissor.top    = 0;
			scissor.bottom = height;
			impl->device_context->RSSetScissorRects( 1, &scissor );

			break;
		}
	#endif
	}

	if( prev_current_context != this )
		prev_current_context->MakeCurrent();
}

void RenderContext::SwapBuffers()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( render_context_impl_index_v< _RenderContextImplOpenGL > ):
		{
			auto impl = std::get_if< _RenderContextImplOpenGL >( &m_impl );

			switch( impl->sub_impl.index() )
			{
				default: break;

			#if _ORB_HAS_WINDOW_API_WIN32
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplWin32 > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplWin32 >( &impl->sub_impl );
					::SwapBuffers( sub_impl->hdc );

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_X11
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplX11 > ):
				{
					auto sub_impl    = std::get_if< _RenderContextImplOpenGL::_SubImplX11 >( &impl->impl );
					auto window_impl = std::get_if< _WindowImplX11 >( sub_impl->parent_window_impl );

					glXSwapBuffers( window_impl->display, window_impl->window );

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_WAYLAND
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplWayland > ):
				{
					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_COCOA
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplCocoa > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplCocoa >( &impl->impl );

					[ [ ( const NSOpenGLView* )sub_impl->view openGLContext ] flushBuffer ];

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_ANDROID
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplAndroid > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplAndroid >( &impl->impl );

					eglSwapBuffers( sub_impl->display, sub_impl->surface );

					break;
				}
			#endif

			#if _ORB_HAS_WINDOW_API_UIKIT
				case( opengl_render_context_impl_index_v< _RenderContextImplOpenGL::_SubImplUIKit > ):
				{
					auto sub_impl = std::get_if< _RenderContextImplOpenGL::_SubImplUIKit >( &impl->impl );

					[ ( GLKView* )sub_impl->view display ];

					break;
				}
			#endif
			}

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( render_context_impl_index_v< _RenderContextImplD3D11 > ):
		{
			auto impl = std::get_if< _RenderContextImplD3D11 >( &m_impl );

			impl->swap_chain->Present( 0, 0 );

			break;
		}
	#endif
	}
}

void RenderContext::Clear( BufferMask mask )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( render_context_impl_index_v< _RenderContextImplOpenGL > ):
		{
			GLbitfield glmask = 0;
			glmask |= ( ( !!( mask & BufferMask::Color ) ) ? GL_COLOR_BUFFER_BIT : 0 );
			glmask |= ( ( !!( mask & BufferMask::Depth ) ) ? GL_DEPTH_BUFFER_BIT : 0 );

			glClear( glmask );

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( render_context_impl_index_v< _RenderContextImplD3D11 > ):
		{
			auto impl = std::get_if< _RenderContextImplD3D11 >( &m_impl );

			if( !!( mask & BufferMask::Color ) ) impl->device_context->ClearRenderTargetView( impl->render_target_view.get(), &impl->clear_color[ 0 ] );
			if( !!( mask & BufferMask::Depth ) ) impl->device_context->ClearDepthStencilView( impl->depth_stencil_view.get(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

			break;
		}
	#endif
	}
}

void RenderContext::SetClearColor( float r, float g, float b )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( render_context_impl_index_v< _RenderContextImplOpenGL > ):
		{
			glClearColor( r, g, b, 1.0f );

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( render_context_impl_index_v< _RenderContextImplD3D11 > ):
		{
			auto impl = std::get_if< _RenderContextImplD3D11 >( &m_impl );
			impl->clear_color.r = r;
			impl->clear_color.g = g;
			impl->clear_color.b = b;
			impl->clear_color.a = 1.0f;

			break;
		}
	#endif
	}
}

RenderContext* RenderContext::GetCurrent()
{
	return current_context;
}

ORB_NAMESPACE_END

#if _ORB_HAS_WINDOW_API_UIKIT
@implementation OrbitGLKViewDelegate

-( void )glkView:( nonnull GLKView* )view drawInRect:( CGRect )rect
{
	/* Unused parameters */
	( void )view;
	( void )rect;
}

@end
#endif
