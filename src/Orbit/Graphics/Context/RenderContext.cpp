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

#include "RenderContext.h"

#include "Orbit/Core/IO/Log.h"
#include "Orbit/Core/Platform/Android/AndroidApp.h"
#include "Orbit/Core/Platform/Windows/ComPtr.h"
#include "Orbit/Core/Platform/Windows/Win32Error.h"
#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Core/Widget/Window.h"
#include "Orbit/Graphics/Platform/iOS/GLKViewDelegate.h"

#include <array>
#include <cstring>

#if defined( ORB_OS_MACOS )
#  include <AppKit/AppKit.h>
#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS
#  include <android/native_window.h>
#endif // ORB_OS_ANDROID

ORB_NAMESPACE_BEGIN

RenderContext::RenderContext( GraphicsAPI api )
	: details_             { }
	, window_resized_      { }
	, window_state_changed_{ }
{
	switch( api )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case GraphicsAPI::OpenGL:
		{
			auto& details = details_.emplace< Private::_RenderContextDetailsOpenGL >();

		#if defined( ORB_OS_WINDOWS )

			auto& window_details = Window::GetInstance().GetPrivateDetails();

			/* Set pixel format */
			{
				details.hdc = GetDC( window_details.hwnd );

				PIXELFORMATDESCRIPTOR desc { };
				desc.nSize      = sizeof( PIXELFORMATDESCRIPTOR );
				desc.nVersion   = 1;
				desc.dwFlags    = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
				desc.iPixelType = PFD_TYPE_RGBA;
				desc.cColorBits = 24;
				desc.cDepthBits = 32;
				desc.iLayerType = PFD_MAIN_PLANE;
				const int format = ChoosePixelFormat( details.hdc, &desc );

				SetPixelFormat( details.hdc, format, &desc );
			}

			/* Create dummy context */
			HGLRC dummy_context = wglCreateContext( details.hdc );
			wglMakeCurrent( details.hdc, dummy_context );

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
					wglChoosePixelFormatARB( details.hdc, format_attributes, nullptr, 1, &pixel_formats, &pixel_format_count );

					const int contextAttributes[]
					{
						0x2091, 4,          // WGL_CONTEXT_MAJOR_VERSION_ARB = 4
						0x2092, 0,          // WGL_CONTEXT_MINOR_VERSION_ARB = 0
						0x9126, 0x00000001, // WGL_CONTEXT_PROFILE_MASK_ARB  = WGL_CONTEXT_CORE_PROFILE_BIT_ARB
						0
					};

					details.hglrc = wglCreateContextAttribsARB( details.hdc, nullptr, contextAttributes );
				}
				else
				{
					details.hglrc = wglCreateContext( details.hdc );
				}
			}

			/* Destroy dummy context */
			wglDeleteContext( dummy_context );

		#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

			auto& window_details = Window::GetInstance().GetPrivateDetails();

			details.gc      = XCreateGC( window_details.display, window_details.window, 0, nullptr );
			details.context = [ & ]
			{
				/* Create render context */
				{
					int screen = DefaultScreen( window_details.display );
					int attribs[]
					{
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
						if( !glXQueryVersion( window_details.display, &major, &minor ) )
							break;
						if( ( major < 1 ) || ( major == 1 && minor < 3 ) )
							break;

						using glXCreateContextAttribsARB_t = GLXContext( * )( Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list );
						glXCreateContextAttribsARB_t glXCreateContextAttribsARB = nullptr;
						glXCreateContextAttribsARB = reinterpret_cast< glXCreateContextAttribsARB_t >( glXGetProcAddressARB( reinterpret_cast< const GLubyte* >( "glXCreateContextAttribsARB" ) ) );
						if( !glXCreateContextAttribsARB )
							break;

						int fb_config_count = 0;
						GLXFBConfig* fb_configs = glXChooseFBConfig( window_details.display, screen, attribs, &fb_config_count );
						if( !fb_configs )
							break;
						if( fb_config_count == 0 )
							break;

						// Choose the best config
						int best_fb_config_idx = -1;
						int best_sample_count  = 0;
						for( int i = 0; i < fb_config_count; ++i )
						{
							XVisualInfo* vi = glXGetVisualFromFBConfig( window_details.display, fb_configs[ i ] );
							if( vi )
							{
								int samples = 0;
								int sample_count = 0;

								glXGetFBConfigAttrib( window_details.display, fb_configs[ i ], GLX_SAMPLE_BUFFERS, &samples );
								glXGetFBConfigAttrib( window_details.display, fb_configs[ i ], GLX_SAMPLES, &sample_count );

								if( samples && sample_count > best_sample_count )
								{
									best_fb_config_idx = i;
									best_sample_count  = sample_count;
								}
							}
							XFree( vi );
						}
						
						if( best_fb_config_idx < 0 )
							break;

						GLXFBConfig best_fb_config = fb_configs[ best_fb_config_idx ];

						XFree( fb_configs );

						int context_attribs[]
						{
							GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
							GLX_CONTEXT_MINOR_VERSION_ARB, 0,
							GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
							None
						};

						return glXCreateContextAttribsARB( window_details.display, best_fb_config, 0, True, context_attribs );

					} while( false );

					// If all else fails, use legacy method
					XVisualInfo* visual_info = glXChooseVisual( window_details.display, screen, attribs );
					return glXCreateContext( window_details.display, visual_info, nullptr, true );
				}
			}();

		#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

			auto& window_details = Window::GetInstance().GetPrivateDetails();

			const NSOpenGLPixelFormatAttribute attribs[]
			{
				NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
				NSOpenGLPFADoubleBuffer,  true,
				NSOpenGLPFAColorSize,     24,
				NSOpenGLPFADepthSize,     24,
				0
			};

			NSOpenGLPixelFormat* pixel_format = [ NSOpenGLPixelFormat alloc ];
			[ pixel_format initWithAttributes:attribs ];

			details.view = [ NSOpenGLView alloc ];
			[ details.view initWithFrame:window_details.window.contentView.frame pixelFormat:pixel_format ];
			[ details.view prepareOpenGL ];
			[ window_details.window.contentView addSubview:details.view ];

		#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

			/* Initialize display */
			{
				details.display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
				eglInitialize( details.display, nullptr, nullptr );
			}

			/* Choose config */
			details.config = EGL_NO_CONFIG_KHR;
			do
			{
				EGLint config_count = 0;
				if( !eglGetConfigs( details.display, nullptr, 0, &config_count ) )
					break;

				std::vector< EGLConfig > configs( static_cast< size_t >( config_count ) );
				if( !eglGetConfigs( details.display, configs.data(), configs.size(), &config_count ) )
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
					eglGetConfigAttrib( details.display, config, EGL_CONFORMANT, &conformant );
					if( ( conformant & required_conformant ) == 0 )
						continue;

					EGLint surface_type = 0;
					eglGetConfigAttrib( details.display, config, EGL_SURFACE_TYPE, &surface_type );
					if( ( surface_type & required_surface_type ) == 0 )
						continue;

					EGLint red_size = 0;
					eglGetConfigAttrib( details.display, config, EGL_RED_SIZE, &red_size );
					if( red_size < best_red_size )
						continue;

					EGLint green_size = 0;
					eglGetConfigAttrib( details.display, config, EGL_RED_SIZE, &green_size );
					if( green_size < best_green_size )
						continue;

					EGLint blue_size = 0;
					eglGetConfigAttrib( details.display, config, EGL_RED_SIZE, &blue_size );
					if( blue_size < best_blue_size )
						continue;

					EGLint alpha_size = 0;
					eglGetConfigAttrib( details.display, config, EGL_RED_SIZE, &alpha_size );
					if( alpha_size < best_alpha_size )
						continue;

					EGLint buffer_size = 0;
					eglGetConfigAttrib( details.display, config, EGL_BUFFER_SIZE, &buffer_size );
					if( buffer_size < best_buffer_size )
						continue;

					EGLint depth_size = 0;
					eglGetConfigAttrib( details.display, config, EGL_DEPTH_SIZE, &depth_size );
					if( depth_size < best_depth_size )
						continue;

					best_red_size    = red_size;
					best_green_size  = green_size;
					best_buffer_size = blue_size;
					best_alpha_size  = alpha_size;
					best_buffer_size = buffer_size;
					best_depth_size  = depth_size;
					details.config   = config;
				}

				EGLint visual_id = 0;
				eglGetConfigAttrib( details.display, details.config, EGL_NATIVE_VISUAL_ID, &visual_id );
				ANativeWindow_setBuffersGeometry( AndroidOnly::app->window, 0, 0, visual_id );

			} while( false );

			/* Create window surface */
			details.surface = eglCreateWindowSurface( details.display, details.config, AndroidOnly::app->window, nullptr );

			/* Create context */
			{
				const EGLint attribs[]
				{
					EGL_CONTEXT_CLIENT_VERSION, 3,
					EGL_NONE,
				};

				details.context = eglCreateContext( details.display, details.config, EGL_NO_CONTEXT, attribs );
			}

		#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

			auto& window_details = Window::GetInstance().GetPrivateDetails();

			ORB_NAMESPACED_OBJC( GLKViewDelegate )* delegate = [ ORB_NAMESPACED_OBJC( GLKViewDelegate ) alloc ];
			[ delegate init ];

			details.context = [ EAGLContext alloc ];
			[ ( EAGLContext* )details.context initWithAPI:kEAGLRenderingAPIOpenGLES3 ];

			details.view = [ GLKView alloc ];
			[ details.view initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
			details.view.drawableColorFormat   = GLKViewDrawableColorFormatRGBA8888;
			details.view.drawableDepthFormat   = GLKViewDrawableDepthFormat24;
			details.view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
			details.view.context               = details.context;
			details.view.delegate              = delegate;
			details.view.enableSetNeedsDisplay = NO;
			[ window_details.ui_window addSubview:details.view ];

		#endif // ORB_OS_IOS

			/* Load functions */
			MakeCurrent();

			glEnable( GL_CULL_FACE );
			glEnable( GL_DEPTH_TEST );
			glCullFace( GL_BACK );
			glFrontFace( GL_CW );

			/* Create version */
			details.version.Init();

			LogInfo( "OpenGL version: %s%d.%d", details.version.IsEmbedded() ? "ES " : "", details.version.GetMajor(), details.version.GetMinor() );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case GraphicsAPI::D3D11:
		{
			auto& details        = details_.emplace< Private::_RenderContextDetailsD3D11 >();
			auto& window_details = Window::GetInstance().GetPrivateDetails();

			constexpr DXGI_FORMAT back_buffer_format  = DXGI_FORMAT_R8G8B8A8_UNORM;
			constexpr DXGI_FORMAT depth_buffer_format = DXGI_FORMAT_D24_UNORM_S8_UINT;

			/* Find the monitor refresh rate */
			ComPtr< IDXGIFactory2 > factory;

		#if defined( NDEBUG )
			CreateDXGIFactory2( 0, __uuidof( IDXGIFactory2 ), reinterpret_cast< void** >( &factory.ptr_ ) );
		#else // NDEBUG
			CreateDXGIFactory2( DXGI_CREATE_FACTORY_DEBUG, __uuidof( IDXGIFactory2 ), reinterpret_cast< void** >( &factory.ptr_ ) );
		#endif // !NDEBUG

			/* Create the device */
			{
				constexpr std::array feature_levels
				{
					D3D_FEATURE_LEVEL_11_1,
					D3D_FEATURE_LEVEL_11_0,
				};

			#if defined( NDEBUG )
				constexpr UINT device_flags = 0;
			#else // NDEBUG
				constexpr UINT device_flags = D3D11_CREATE_DEVICE_DEBUG;
			#endif // !NDEBUG

				D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, device_flags, feature_levels.data(), static_cast< UINT >( feature_levels.size() ), D3D11_SDK_VERSION, &details.device.ptr_, NULL, &details.device_context.ptr_ );
			}

			/* Create the swap chain */
			{
				RECT window_rect;
				GetWindowRect( window_details.hwnd, &window_rect );

				DXGI_SWAP_CHAIN_DESC1 desc { };
				desc.Width                  = ( window_rect.right - window_rect.left );
				desc.Height                 = ( window_rect.bottom - window_rect.top );
				desc.Format                 = back_buffer_format;
				desc.Stereo                 = false;
				desc.SampleDesc.Count       = 1;
				desc.SampleDesc.Quality     = 0;
				desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				desc.BufferCount            = 1;
				desc.Scaling                = DXGI_SCALING_STRETCH;
				desc.SwapEffect             = DXGI_SWAP_EFFECT_DISCARD;
				desc.AlphaMode              = DXGI_ALPHA_MODE_IGNORE;
				desc.Flags                  = 0;

				ORB_CHECK_HRESULT( factory->CreateSwapChainForHwnd( details.device.ptr_, window_details.hwnd, &desc, NULL, NULL, &details.swap_chain.ptr_ ) );
			}

			/* Create the render target */
			{
				ComPtr< ID3D11Texture2D > back_buffer;
				details.swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &back_buffer.ptr_ ) );
				details.device->CreateRenderTargetView( back_buffer.ptr_, NULL, &details.render_target_view.ptr_ );
			}

			/* Create the depth stencil */
			{
				RECT rect { };
				GetWindowRect( window_details.hwnd, &rect );

				D3D11_TEXTURE2D_DESC buffer_desc { };
				buffer_desc.Width            = ( rect.right - rect.left );
				buffer_desc.Height           = ( rect.bottom - rect.top );
				buffer_desc.MipLevels        = 1;
				buffer_desc.ArraySize        = 1;
				buffer_desc.Format           = depth_buffer_format;
				buffer_desc.SampleDesc.Count = 1;
				buffer_desc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

				details.device->CreateTexture2D( &buffer_desc, NULL, &details.depth_stencil_buffer.ptr_ );

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

				details.device->CreateDepthStencilState( &state_desc, &details.depth_stencil_state.ptr_ );
				details.device_context->OMSetDepthStencilState( details.depth_stencil_state.ptr_, 1 );
					
				D3D11_DEPTH_STENCIL_VIEW_DESC view_desc { };
				view_desc.Format        = depth_buffer_format;
				view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

				details.device->CreateDepthStencilView( details.depth_stencil_buffer.ptr_, &view_desc, &details.depth_stencil_view.ptr_ );
				details.device_context->OMSetRenderTargets( 1, &details.render_target_view.ptr_, details.depth_stencil_view.ptr_ );
			}

			/* Create rasterizer */
			{
				D3D11_RASTERIZER_DESC desc { };
				desc.CullMode              = D3D11_CULL_BACK;
				desc.DepthClipEnable       = true;
				desc.FillMode              = D3D11_FILL_SOLID;
				desc.FrontCounterClockwise = false;
				desc.ScissorEnable         = true;

				details.device->CreateRasterizerState( &desc, &details.rasterizer_state.ptr_ );
				details.device_context->RSSetState( details.rasterizer_state.ptr_ );
			}

			/* Set default topology */
			details.device_context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			break;
		}

	#endif // ORB_HAS_D3D11

	}

	/* Resize context when window is updated */
	window_resized_ = Window::GetInstance().Subscribe( [ this ]( const WindowResized& e )
		{
			Resize( e.width, e.height );
		}
	);

	/* Disable rendering when minimized */
	window_state_changed_ = Window::GetInstance().Subscribe( [ this ]( const WindowStateChanged& e )
		{
			if( e.state == WindowState::Suspend )
			{
				Resize( 0, 0 );
			}
		}
	);

	/* make current */
	MakeCurrent();
}

RenderContext::~RenderContext()
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& details = std::get< Private::_RenderContextDetailsOpenGL >( details_ );

		#if defined( ORB_OS_WINDOWS )

			HWND hwnd = WindowFromDC( details.hdc );

			wglMakeCurrent( NULL, NULL );
			wglDeleteContext( details.hglrc );
			ReleaseDC( hwnd, details.hdc );

		#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

			auto& window_details = Window::GetInstance().GetPrivateDetails();

			glXMakeCurrent( window_details.display, None, nullptr );
			glXDestroyContext( window_details.display, details.context );
			XFreeGC( window_details.display, details.gc );

		#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

			[ NSOpenGLContext clearCurrentContext ];
			[ details.view removeFromSuperview ];
			[ details.view dealloc ];

		#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

			eglMakeCurrent( details.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
			eglDestroyContext( details.display, details.context );
			eglDestroySurface( details.display, details.surface );
			eglTerminate( details.display );

		#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

			[ EAGLContext setCurrentContext:nullptr ];
			[ details.view dealloc ];
			[ details.context dealloc ];

		#endif // ORB_OS_IOS

			break;
		}

	#endif // ORB_HAS_OPENGL

	}
}

bool RenderContext::MakeCurrent()
{

#if( ORB_HAS_OPENGL )

	if( details_.index() == unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > )
	{
		auto& details = std::get< Private::_RenderContextDetailsOpenGL >( details_ );

	#if defined( ORB_OS_WINDOWS )

		if( !wglMakeCurrent( details.hdc, details.hglrc ) )
			return false;

	#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

		auto& window_details = Window::GetInstance().GetPrivateDetails();

		if( !glXMakeCurrent( window_details.display, window_details.window, details.context ) )
			return false;

	#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

		[ [ ( const NSOpenGLView* )details.view openGLContext ] makeCurrentContext ];

	#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

		eglMakeCurrent( details.display, details.surface, details.surface, details.context );

	#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

		[ EAGLContext setCurrentContext:details.context ];

	#endif // ORB_OS_IOS

	}

#endif // ORB_HAS_OPENGL

	return true;
}

void RenderContext::Resize( uint32_t width, uint32_t height )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{

		#if defined( ORB_OS_ANDROID )

			auto& details = std::get< Private::_RenderContextDetailsOpenGL >( details_ );

			eglMakeCurrent( details.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );

			if( details.surface != EGL_NO_SURFACE )
				eglDestroySurface( details.display, details.surface );

			details.surface = eglCreateWindowSurface( details.display, details.config, AndroidOnly::app->window, nullptr );

			eglMakeCurrent( details.display, details.surface, details.surface, details.context );

		#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

			auto& details = std::get< Private::_RenderContextDetailsOpenGL >( details_ );

			details.view.layer.frame = CGRectMake( 0.f, 0.f, width, height );

		#endif // ORB_OS_IOS

			glViewport( 0, 0, width, height );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = std::get< Private::_RenderContextDetailsD3D11 >( details_ );

			details.device_context->OMSetRenderTargets( 0, nullptr, nullptr );
			details.device_context->ClearState();
			details.device_context->Flush();

			D3D11_TEXTURE2D_DESC depth_stencil_buffer_desc { };
			details.depth_stencil_buffer->GetDesc( &depth_stencil_buffer_desc );

			D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc { };
			details.depth_stencil_view->GetDesc( &depth_stencil_view_desc );

			details.render_target_view   = nullptr;
			details.depth_stencil_buffer = nullptr;
			details.depth_stencil_view   = nullptr;

			DXGI_SWAP_CHAIN_DESC1 swap_chain_desc { };
			details.swap_chain->GetDesc1( &swap_chain_desc );

			if( ( width > 0 ) && ( height > 0 ) )
			{
				swap_chain_desc.Width  = width;
				swap_chain_desc.Height = height;
			}

			details.swap_chain->ResizeBuffers( 1, swap_chain_desc.Width, swap_chain_desc.Height, swap_chain_desc.Format, swap_chain_desc.Flags );

			/* Recreate render target */
			{
				ComPtr< ID3D11Texture2D > back_buffer;

				details.render_target_view = nullptr;

				details.swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &back_buffer.ptr_ ) );
				details.device->CreateRenderTargetView( back_buffer.ptr_, nullptr, &details.render_target_view.ptr_ );
			}

			/* Recreate depth stencil */
			{
				depth_stencil_buffer_desc.Width  = swap_chain_desc.Width;
				depth_stencil_buffer_desc.Height = swap_chain_desc.Height;

				details.depth_stencil_buffer = nullptr;
				details.device->CreateTexture2D( &depth_stencil_buffer_desc, NULL, &details.depth_stencil_buffer.ptr_ );

				details.depth_stencil_view = nullptr;
				details.device->CreateDepthStencilView( details.depth_stencil_buffer.ptr_, &depth_stencil_view_desc, &details.depth_stencil_view.ptr_ );
			}

			details.device_context->OMSetRenderTargets( 1, &details.render_target_view.ptr_, details.depth_stencil_view.ptr_ );
			details.device_context->OMSetDepthStencilState( details.depth_stencil_state.ptr_, 0 );
			details.device_context->RSSetState( details.rasterizer_state.ptr_ );
			details.device_context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			D3D11_VIEWPORT viewport { };
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width    = static_cast< FLOAT >( width );
			viewport.Height   = static_cast< FLOAT >( height );
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			details.device_context->RSSetViewports( 1, &viewport );

			RECT scissor { };
			scissor.left   = 0;
			scissor.right  = width;
			scissor.top    = 0;
			scissor.bottom = height;
			details.device_context->RSSetScissorRects( 1, &scissor );

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

void RenderContext::SwapBuffers( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{

		#if defined( ORB_OS_WINDOWS )

			auto& details = std::get< Private::_RenderContextDetailsOpenGL >( details_ );

			::SwapBuffers( details.hdc );

		#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

			auto& window_details = Window::GetInstance().GetPrivateDetails();

			glXSwapBuffers( window_details.display, window_details.window );

		#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

			auto& details = std::get< Private::_RenderContextDetailsOpenGL >( details_ );

			[ [ ( const NSOpenGLView* )details.view openGLContext ] flushBuffer ];

		#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

			auto& details = std::get< Private::_RenderContextDetailsOpenGL >( details_ );

			eglSwapBuffers( details.display, details.surface );

		#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

			auto& details = std::get< Private::_RenderContextDetailsOpenGL >( details_ );

			[ details.view display ];

		#endif // ORB_OS_IOS

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = std::get< Private::_RenderContextDetailsD3D11 >( details_ );

			details.swap_chain->Present( 0, 0 );

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

void RenderContext::Clear( BufferMask mask )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			GLbitfield glmask = 0;
			glmask |= ( ( !!( mask & BufferMask::Color ) ) ? GL_COLOR_BUFFER_BIT : 0 );
			glmask |= ( ( !!( mask & BufferMask::Depth ) ) ? GL_DEPTH_BUFFER_BIT : 0 );

			glClear( glmask );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = std::get< Private::_RenderContextDetailsD3D11 >( details_ );

			if( !!( mask & BufferMask::Color ) )
				details.device_context->ClearRenderTargetView( details.render_target_view.ptr_, &details.clear_color[ 0 ] );

			if( !!( mask & BufferMask::Depth ) )
				details.device_context->ClearDepthStencilView( details.depth_stencil_view.ptr_, D3D11_CLEAR_DEPTH, 1.0f, 0 );

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

void RenderContext::SetClearColor( float r, float g, float b )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			glClearColor( r, g, b, 1.0f );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = std::get< Private::_RenderContextDetailsD3D11 >( details_ );
			details.clear_color.r = r;
			details.clear_color.g = g;
			details.clear_color.b = b;
			details.clear_color.a = 1.0f;

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

ORB_NAMESPACE_END
