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

#if( ORB_HAS_D3D11 )
constexpr DXGI_FORMAT back_buffer_format  = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr DXGI_FORMAT depth_buffer_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
#endif

RenderContext::RenderContext( GraphicsAPI api )
	: m_data                { }
	, m_resize_subscription { }
{
	switch( api )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case GraphicsAPI::OpenGL:
		{
			auto& data        = m_data.emplace< Private::_RenderContextDataOpenGL >();
			auto& window_data = Window::GetInstance().GetPrivateData();

		#if defined( ORB_OS_WINDOWS )

			/* Set pixel format */
			{
				data.hdc = GetDC( window_data.hwnd );

				PIXELFORMATDESCRIPTOR desc { };
				desc.nSize      = sizeof( PIXELFORMATDESCRIPTOR );
				desc.nVersion   = 1;
				desc.dwFlags    = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
				desc.iPixelType = PFD_TYPE_RGBA;
				desc.cColorBits = 24;
				desc.cDepthBits = 32;
				desc.iLayerType = PFD_MAIN_PLANE;
				const int format = ChoosePixelFormat( data.hdc, &desc );

				SetPixelFormat( data.hdc, format, &desc );
			}

			/* Create dummy context */
			HGLRC dummy_context = wglCreateContext( data.hdc );
			wglMakeCurrent( data.hdc, dummy_context );

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
					wglChoosePixelFormatARB( data.hdc, format_attributes, nullptr, 1, &pixel_formats, &pixel_format_count );

					const int contextAttributes[]
					{
						0x2091, 4,          // WGL_CONTEXT_MAJOR_VERSION_ARB = 4
						0x2092, 0,          // WGL_CONTEXT_MINOR_VERSION_ARB = 0
						0x9126, 0x00000001, // WGL_CONTEXT_PROFILE_MASK_ARB  = WGL_CONTEXT_CORE_PROFILE_BIT_ARB
						0
					};

					data.hglrc = wglCreateContextAttribsARB( data.hdc, nullptr, contextAttributes );
				}
				else
				{
					data.hglrc = wglCreateContext( data.hdc );
				}
			}

			/* Destroy dummy context */
			wglDeleteContext( dummy_context );

		#elif defined( ORB_OS_LINUX )

			sub_data->gc         = XCreateGC( window_data.display, window_data.window, 0, nullptr );
			sub_data->glxContext = [ & ]
			{
				/* Create render context */
				{
					int screen = DefaultScreen( window_data.display );
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
						if( !glXQueryVersion( window_data->display, &major, &minor ) )
							break;
						if( ( major < 1 ) || ( major == 1 && minor < 3 ) )
							break;

						using glXCreateContextAttribsARB_t = GLXContext( * )( Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list );
						glXCreateContextAttribsARB_t glXCreateContextAttribsARB = nullptr;
						glXCreateContextAttribsARB = reinterpret_cast< glXCreateContextAttribsARB_t >( glXGetProcAddressARB( reinterpret_cast< const GLubyte* >( "glXCreateContextAttribsARB" ) ) );
						if( !glXCreateContextAttribsARB )
							break;

						int fb_config_count = 0;
						GLXFBConfig* fb_configs = glXChooseFBConfig( window_data.display, screen, attribs, &fb_config_count );
						if( !fb_configs )
							break;
						if( fb_config_count == 0 )
							break;

						// Choose the best config
						int best_fb_config_idx = 0;
						int best_sample_count = 0;
						for( int i = 0; i < fb_config_count; ++i )
						{
							XVisualInfo* vi = glXGetVisualFromFBConfig( window_data.display, fb_configs[ i ] );
							if( vi )
							{
								int samples = 0;
								int sample_count = 0;

								glXGetFBConfigAttrib( window_data.display, fb_configs[ i ], GLX_SAMPLE_BUFFERS, &samples );
								glXGetFBConfigAttrib( window_data.display, fb_configs[ i ], GLX_SAMPLES, &sample_count );

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

						return glXCreateContextAttribsARB( window_data.display, best_fb_config, 0, True, context_attribs );

					} while( false );

					// If all else fails, use legacy method
					XVisualInfo* visual_info = glXChooseVisual( window_data.display, screen, attribs );
					return glXCreateContext( window_data.display, visual_info, nullptr, true );
				}
			}();

		#elif defined( ORB_OS_MACOS )

			const NSOpenGLPixelFormatAttribute attribs[]
			{
				NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
				NSOpenGLPFADoubleBuffer,  true,
				NSOpenGLPFAColorSize,     24,
				NSOpenGLPFADepthSize,     24,
				0
			};

			NSWindow*            ns_indow     = ( NSWindow* )window_data->ns_window;
			NSOpenGLPixelFormat* pixel_format = [ NSOpenGLPixelFormat alloc ];
			[ pixel_format initWithAttributes:attribs ];

			sub_data->view = [ NSOpenGLView alloc ];
			[ ( NSOpenGLView* )sub_data->view initWithFrame:ns_window.contentView.frame pixelFormat:pixel_format ];
			[ ( NSOpenGLView* )sub_data->view prepareOpenGL ];
			[ ns_window.contentView addSubview:( NSOpenGLView* )sub_data->view ];

		#elif defined( ORB_OS_ANDROID )

			/* Initialize display */
			{
				data->display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
				eglInitialize( data->display, nullptr, nullptr );
			}

			/* Choose config */
			data->config = EGL_NO_CONFIG_KHR;
			do
			{
				EGLint config_count = 0;
				if( !eglGetConfigs( data->display, nullptr, 0, &config_count ) )
					break;

				std::vector< EGLConfig > configs( static_cast< size_t >( config_count ) );
				if( !eglGetConfigs( data->display, configs.data(), configs.size(), &config_count ) )
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
					eglGetConfigAttrib( data->display, config, EGL_CONFORMANT, &conformant );
					if( ( conformant & required_conformant ) == 0 )
						continue;

					EGLint surface_type = 0;
					eglGetConfigAttrib( data->display, config, EGL_SURFACE_TYPE, &surface_type );
					if( ( surface_type & required_surface_type ) == 0 )
						continue;

					EGLint red_size = 0;
					eglGetConfigAttrib( data->display, config, EGL_RED_SIZE, &red_size );
					if( red_size < best_red_size )
						continue;

					EGLint green_size = 0;
					eglGetConfigAttrib( data->display, config, EGL_RED_SIZE, &green_size );
					if( green_size < best_green_size )
						continue;

					EGLint blue_size = 0;
					eglGetConfigAttrib( data->display, config, EGL_RED_SIZE, &blue_size );
					if( blue_size < best_blue_size )
						continue;

					EGLint alpha_size = 0;
					eglGetConfigAttrib( data->display, config, EGL_RED_SIZE, &alpha_size );
					if( alpha_size < best_alpha_size )
						continue;

					EGLint buffer_size = 0;
					eglGetConfigAttrib( data->display, config, EGL_BUFFER_SIZE, &buffer_size );
					if( buffer_size < best_buffer_size )
						continue;

					EGLint depth_size = 0;
					eglGetConfigAttrib( data->display, config, EGL_DEPTH_SIZE, &depth_size );
					if( depth_size < best_depth_size )
						continue;

					best_red_size    = red_size;
					best_green_size  = green_size;
					best_buffer_size = blue_size;
					best_alpha_size  = alpha_size;
					best_buffer_size = buffer_size;
					best_depth_size  = depth_size;
					data->config     = config;
				}

				EGLint visual_id = 0;
				eglGetConfigAttrib( data->display, sub_data->config, EGL_NATIVE_VISUAL_ID, &visual_id );
				ANativeWindow_setBuffersGeometry( AndroidOnly::app->window, 0, 0, visual_id );

			} while( false );

			/* Create window surface */
			data->surface = eglCreateWindowSurface( data->display, data->config, AndroidOnly::app->window, nullptr );

			/* Create context */
			{
				const EGLint attribs[]
				{
					EGL_CONTEXT_CLIENT_VERSION, 3,
					EGL_NONE,
				};

				data->context = eglCreateContext( data->display, data->config, EGL_NO_CONTEXT, attribs );
			}

		#elif defined( ORB_OS_IOS )

			OrbitGLKViewDelegate* delegate = [ OrbitGLKViewDelegate alloc ];
			[ delegate init ];

			data->context = [ EAGLContext alloc ];
			[ ( EAGLContext* )data->context initWithAPI:kEAGLRenderingAPIOpenGLES3 ];

			data->view = [ GLKView alloc ];
			[ ( GLKView* )data->view initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
			( ( GLKView* )data->view ).context               = ( EAGLContext* )data->context;
			( ( GLKView* )data->view ).delegate              = delegate;
			( ( GLKView* )data->view ).enableSetNeedsDisplay = NO;
			[ ( UIWindow* )window_data.ui_window addSubview:( GLKView* )data->view ];

		#endif

			/* Load functions */
			MakeCurrent();

			glEnable( GL_CULL_FACE );
			glEnable( GL_DEPTH_TEST );
			glCullFace( GL_BACK );
			glFrontFace( GL_CW );

			const char* opengl_version = reinterpret_cast< const char* >( glGetString( GL_VERSION ) );

			auto check_digit = [ & ]( Version* out )
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
				data.embedded = true;
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

			data.opengl_version = parse_version();

			LogInfo( Format( "OpenGL version: %s %d.%d", data.embedded ? "ES" : "", data.opengl_version.major, data.opengl_version.minor ) );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case GraphicsAPI::D3D11:
		{
			auto& data        = m_data.emplace< Private::_RenderContextDataD3D11 >();
			auto& window_data = Window::GetInstance().GetPrivateData();

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
					HMONITOR    monitor = MonitorFromWindow( window_data.hwnd, MONITOR_DEFAULTTONEAREST );
					MONITORINFO monitor_info;
					monitor_info.cbSize = sizeof( MONITORINFO );
					GetMonitorInfoA( monitor, &monitor_info );

					monitor_width  = ( monitor_info.rcMonitor.right - monitor_info.rcMonitor.left );
					monitor_height = ( monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top );
				}

				std::vector< DXGI_MODE_DESC > display_modes;
				{ /* Get display modes. */
					UINT num_display_modes;
					output->GetDisplayModeList( back_buffer_format, DXGI_ENUM_MODES_INTERLACED, &num_display_modes, nullptr );
					display_modes.resize( num_display_modes );
					output->GetDisplayModeList( back_buffer_format, DXGI_ENUM_MODES_INTERLACED, &num_display_modes, &display_modes[ 0 ] );
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
				GetWindowRect( window_data.hwnd, &window_rect );

				DXGI_SWAP_CHAIN_DESC desc { };
				desc.BufferDesc.Width       = ( window_rect.right - window_rect.left );
				desc.BufferDesc.Height      = ( window_rect.bottom - window_rect.top );
				desc.BufferDesc.RefreshRate = refreshRate;
				desc.BufferDesc.Format      = back_buffer_format;
				desc.SampleDesc.Count       = 1;
				desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				desc.BufferCount            = 1;
				desc.OutputWindow           = window_data.hwnd;
				desc.Windowed               = true;
				desc.SwapEffect             = DXGI_SWAP_EFFECT_DISCARD;

				IDXGISwapChain* swap_chain;
				D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, kDeviceFlags, kFeatureLevels.data(), static_cast< UINT >( kFeatureLevels.size() ), D3D11_SDK_VERSION, &desc, &swap_chain, NULL, NULL, NULL );
				data.swap_chain.reset( swap_chain );
			}

			/* Get the device */
			{
				void* device;
				data.swap_chain->GetDevice( __uuidof( ID3D11Device ), &device );
				data.device.reset( static_cast< ID3D11Device* >( device ) );
			}

			/* Get the device context */
			{
				ID3D11DeviceContext* deviceContext;
				data.device->GetImmediateContext( &deviceContext );
				data.device_context.reset( deviceContext );
			}

			/* Create the render target */
			{
				ID3D11Texture2D* back_buffer;
				data.swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &back_buffer ) );
				ID3D11RenderTargetView* renderTargetView;
				data.device->CreateRenderTargetView( back_buffer, NULL, &renderTargetView );
				data.render_target_view.reset( renderTargetView );
				back_buffer->Release();
			}

			/* Create the depth stencil */
			{
				RECT rect { };
				GetWindowRect( window_data.hwnd, &rect );

				D3D11_TEXTURE2D_DESC buffer_desc { };
				buffer_desc.Width            = ( rect.right - rect.left );
				buffer_desc.Height           = ( rect.bottom - rect.top );
				buffer_desc.MipLevels        = 1;
				buffer_desc.ArraySize        = 1;
				buffer_desc.Format           = depth_buffer_format;
				buffer_desc.SampleDesc.Count = 1;
				buffer_desc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

				ID3D11Texture2D* depth_stencil_buffer;
				data.device->CreateTexture2D( &buffer_desc, NULL, &depth_stencil_buffer );
				data.depth_stencil_buffer.reset( depth_stencil_buffer );

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
				data.device->CreateDepthStencilState( &state_desc, &depth_stencil_state );
				data.device_context->OMSetDepthStencilState( depth_stencil_state, 1 );
				data.depth_stencil_state.reset( depth_stencil_state );
					
				D3D11_DEPTH_STENCIL_VIEW_DESC view_desc { };
				view_desc.Format        = depth_buffer_format;
				view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

				ID3D11DepthStencilView* depth_stencil_view;
				data.device->CreateDepthStencilView( depth_stencil_buffer, &view_desc, &depth_stencil_view );

				ID3D11RenderTargetView* render_target_views[] = { data.render_target_view.get() };
				data.device_context->OMSetRenderTargets( 1, render_target_views, depth_stencil_view );
				data.depth_stencil_view.reset( depth_stencil_view );
			}

			/* Create rasterizer */
			{
				D3D11_RASTERIZER_DESC desc { };
				desc.CullMode              = D3D11_CULL_BACK;
				desc.DepthClipEnable       = true;
				desc.FillMode              = D3D11_FILL_SOLID;
				desc.FrontCounterClockwise = false;
				desc.ScissorEnable         = true;

				ID3D11RasterizerState* rasterizer_state;
				data.device->CreateRasterizerState( &desc, &rasterizer_state );
				data.device_context->RSSetState( rasterizer_state );
				data.rasterizer_state.reset( rasterizer_state );
			}

			/* Set default topology */
			data.device_context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			break;
		}

	#endif

	}

	/* Resize context when window is updated */
	m_resize_subscription = Window::GetInstance().Subscribe( [ this ]( const WindowResized& e )
		{
			Resize( e.width, e.height );
		}
	);

	/* make current */
	MakeCurrent();
}

RenderContext::~RenderContext()
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{
			auto& data = std::get< Private::_RenderContextDataOpenGL >( m_data );

		#if defined( ORB_OS_WINDOWS )

			HWND hwnd = WindowFromDC( data.hdc );

			wglMakeCurrent( NULL, NULL );
			wglDeleteContext( data.hglrc );
			ReleaseDC( hwnd, data.hdc );

		#elif defined( ORB_OS_LINUX )

			glXMakeCurrent( data.display, None, nullptr );
			glXDestroyContext( data.display, data.context );
			XFreeGC( data.display, data.gc );

		#elif defined( ORB_OS_MACOS )

			[ NSOpenGLContext clearCurrentContext ];
			[ ( const NSOpenGLView* )data.view removeFromSuperview ];
			[ ( const NSOpenGLView* )data.view dealloc ];

		#elif defined( ORB_OS_ANDROID )

			eglMakeCurrent( data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
			eglDestroyContext( data.display, data.context );
			eglDestroySurface( data.display, data.surface );
			eglTerminate( data.display );

		#elif defined( ORB_OS_IOS )

			[ EAGLContext setCurrentContext:nullptr ];
			[ ( GLKView* )data.view dealloc ];
			[ ( EAGLContext* )data.context dealloc ];

		#endif

			break;
		}

	#endif

	}
}

bool RenderContext::MakeCurrent()
{

#if( ORB_HAS_OPENGL )

	if( m_data.index() == unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > )
	{
		auto& data = std::get< Private::_RenderContextDataOpenGL >( m_data );

	#if defined( ORB_OS_WINDOWS )

		if( !wglMakeCurrent( data.hdc, data.hglrc ) )
			return false;

	#elif defined( ORB_OS_LINUX )

		if( !glXMakeCurrent( data.window_data->display, data.window_data->window, data.context ) )
			return false;

	#elif defined( ORB_OS_MACOS )

		[ [ ( const NSOpenGLView* )data.view openGLContext ] makeCurrentContext ];

	#elif defined( ORB_OS_ANDROID )

		eglMakeCurrent( data.display, data.surface, data.surface, data.context );

	#elif defined( ORB_OS_IOS )

		[ EAGLContext setCurrentContext:( EAGLContext* )data.context ];

	#endif

	}

#endif

	return true;
}

void RenderContext::Resize( uint32_t width, uint32_t height )
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{

		#if defined( ORB_OS_ANDROID )

			auto& data = std::get< Private::_RenderContextDataOpenGL >( m_data );

			eglMakeCurrent( data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );

			if( data.surface != EGL_NO_SURFACE )
				eglDestroySurface( data.display, data.surface );

			data.surface = eglCreateWindowSurface( data.display, data.eglConfig, AndroidOnly::app->window, nullptr );

		#elif defined( ORB_OS_IOS )

			( ( GLKView* )data.view ).layer.frame = CGRectMake( 0.f, 0.f, width, height );

		#endif

			glViewport( 0, 0, width, height );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
		{
			auto& data = std::get< Private::_RenderContextDataD3D11 >( m_data );

			data.device_context->OMSetRenderTargets( 0, nullptr, nullptr );
			data.device_context->ClearState();
			data.device_context->Flush();

			D3D11_TEXTURE2D_DESC depth_stencil_buffer_desc { };
			data.depth_stencil_buffer->GetDesc( &depth_stencil_buffer_desc );

			D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc { };
			data.depth_stencil_view->GetDesc( &depth_stencil_view_desc );

			data.render_target_view.reset();
			data.depth_stencil_buffer.reset();
			data.depth_stencil_view.reset();

			DXGI_SWAP_CHAIN_DESC swap_chain_desc { };
			data.swap_chain->GetDesc( &swap_chain_desc );
			swap_chain_desc.BufferDesc.Width  = width;
			swap_chain_desc.BufferDesc.Height = height;
			data.swap_chain->ResizeBuffers( 1, swap_chain_desc.BufferDesc.Width, swap_chain_desc.BufferDesc.Height, swap_chain_desc.BufferDesc.Format, swap_chain_desc.Flags );

			/* Recreate render target */
			{
				ID3D11Texture2D* back_buffer;
				data.swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &back_buffer ) );
				ID3D11RenderTargetView* renderTargetView;
				data.device->CreateRenderTargetView( back_buffer, nullptr, &renderTargetView );
				data.render_target_view.reset( renderTargetView );
				back_buffer->Release();
			}

			/* Recreate depth stencil */
			{
				depth_stencil_buffer_desc.Width  = width;
				depth_stencil_buffer_desc.Height = height;

				ID3D11Texture2D* depth_stencil_buffer;
				data.device->CreateTexture2D( &depth_stencil_buffer_desc, NULL, &depth_stencil_buffer );
				data.depth_stencil_buffer.reset( depth_stencil_buffer );

				ID3D11DepthStencilView* depth_stencil_view;
				data.device->CreateDepthStencilView( depth_stencil_buffer, &depth_stencil_view_desc, &depth_stencil_view );
				data.depth_stencil_view.reset( depth_stencil_view );
			}

			ID3D11RenderTargetView* render_target_views[] = { data.render_target_view.get() };
			data.device_context->OMSetRenderTargets( 1, render_target_views, data.depth_stencil_view.get() );
			data.device_context->OMSetDepthStencilState( data.depth_stencil_state.get(), 0 );
			data.device_context->RSSetState( data.rasterizer_state.get() );
			data.device_context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			D3D11_VIEWPORT viewport { };
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width    = static_cast< FLOAT >( width );
			viewport.Height   = static_cast< FLOAT >( height );
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			data.device_context->RSSetViewports( 1, &viewport );

			RECT scissor { };
			scissor.left   = 0;
			scissor.right  = width;
			scissor.top    = 0;
			scissor.bottom = height;
			data.device_context->RSSetScissorRects( 1, &scissor );

			break;
		}

	#endif

	}
}

void RenderContext::SwapBuffers()
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{
			auto& data = std::get< Private::_RenderContextDataOpenGL >( m_data );

		#if defined( ORB_OS_WINDOWS )

			::SwapBuffers( data.hdc );

		#elif defined( ORB_OS_LINUX )

			glXSwapBuffers( data.window_data->display, data.window_data->window );

		#elif defined( ORB_OS_MACOS )

			[ [ ( const NSOpenGLView* )data.view openGLContext ] flushBuffer ];

		#elif defined( ORB_OS_ANDROID )

			eglSwapBuffers( data.display, data.surface );

		#elif defined( ORB_OS_IOS )

			[ ( GLKView* )data.view display ];

		#endif

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
		{
			auto& data = std::get< Private::_RenderContextDataD3D11 >( m_data );

			data.swap_chain->Present( 0, 0 );

			break;
		}

	#endif

	}
}

void RenderContext::Clear( BufferMask mask )
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{
			GLbitfield glmask = 0;
			glmask |= ( ( !!( mask & BufferMask::Color ) ) ? GL_COLOR_BUFFER_BIT : 0 );
			glmask |= ( ( !!( mask & BufferMask::Depth ) ) ? GL_DEPTH_BUFFER_BIT : 0 );

			glClear( glmask );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
		{
			auto& data = std::get< Private::_RenderContextDataD3D11 >( m_data );

			if( !!( mask & BufferMask::Color ) )
				data.device_context->ClearRenderTargetView( data.render_target_view.get(), &data.clear_color[ 0 ] );

			if( !!( mask & BufferMask::Depth ) )
				data.device_context->ClearDepthStencilView( data.depth_stencil_view.get(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

			break;
		}

	#endif

	}
}

void RenderContext::SetClearColor( float r, float g, float b )
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{
			glClearColor( r, g, b, 1.0f );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
		{
			auto& data = std::get< Private::_RenderContextDataD3D11 >( m_data );
			data.clear_color.r = r;
			data.clear_color.g = g;
			data.clear_color.b = b;
			data.clear_color.a = 1.0f;

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END

#if defined( ORB_OS_IOS )

@implementation OrbitGLKViewDelegate

-( void )glkView:( nonnull GLKView* )view drawInRect:( CGRect )rect
{
	/* Unused parameters */
	( void )view;
	( void )rect;
}

@end

#endif
