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

#include "render_context.h"

#include <array>

#include "orbit/core/android_app.h"
#include "orbit/core/log.h"
#include "orbit/core/memory.h"
#include "orbit/core/window.h"
#include "orbit/core/utility.h"

#if defined( ORB_OS_ANDROID )
  #include <android/native_window.h>
#endif

#if __ORB_HAS_WINDOW_API_UIKIT
@interface ORBGLKViewDelegate : UIResponder< GLKViewDelegate >
@end
#endif

namespace orb
{
	static render_context* CurrentContext = nullptr;

	template< typename T >
	constexpr auto window_impl_index_v = unique_index_v< T, window_impl >;

	template< typename T >
	constexpr auto render_context_impl_index_v = unique_index_v< T, render_context_impl >;

#if __ORB_HAS_GRAPHICS_API_D3D11
	constexpr DXGI_FORMAT kBackBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr DXGI_FORMAT kDepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
#endif
#if __ORB_HAS_GRAPHICS_API_OPENGL
	template< typename T >
	constexpr auto opengl_render_context_impl_index_v = unique_index_v< T, __render_context_impl_opengl::__impl >;
#endif

	render_context::render_context( window& parentWindow, graphics_api api )
		: m_impl{ }
		, m_resizeSubscription{ }
	{
		switch( api )
		{
			default:
			{
				( void )parentWindow;
				break;
			}

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case graphics_api::OpenGL:
			{
				auto parentWindowImpl = parentWindow.get_impl_ptr();
				auto impl             = std::addressof( m_impl.emplace< __render_context_impl_opengl >() );
				switch( parentWindowImpl->index() )
				{
					default: break;

				#if __ORB_HAS_WINDOW_API_WIN32
					case( window_impl_index_v< __window_impl_win32 > ):
					{
						auto implGl = std::addressof( impl->impl.emplace< __render_context_impl_opengl::__impl_win32 >() );
						implGl->parentWindowImpl = parentWindowImpl;

						/* Set pixel format */
						{
							implGl->deviceContext = GetDC( std::get_if< __window_impl_win32 >( implGl->parentWindowImpl )->hwnd );

							PIXELFORMATDESCRIPTOR desc{ };
							desc.nSize      = sizeof( PIXELFORMATDESCRIPTOR );
							desc.nVersion   = 1;
							desc.dwFlags    = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
							desc.iPixelType = PFD_TYPE_RGBA;
							desc.cColorBits = 24;
							desc.cDepthBits = 32;
							desc.iLayerType = PFD_MAIN_PLANE;
							const int format = ChoosePixelFormat( implGl->deviceContext, &desc );

							SetPixelFormat( implGl->deviceContext, format, &desc );
						}

						/* Create dummy context */
						HGLRC dummyContext = wglCreateContext( implGl->deviceContext );
						wglMakeCurrent( implGl->deviceContext, dummyContext );

						/* Create persistent context */
						{
							using wglChoosePixelFormatARB_t = BOOL( WINAPI* )( HDC hdc, const int* iAttribs, const FLOAT* fAttribs, UINT maxFormats, int* formats, UINT* numFormats );
							auto  wglChoosePixelFormatARB   = reinterpret_cast< wglChoosePixelFormatARB_t    >( wglGetProcAddress( "wglChoosePixelFormatARB"    ) );

							using wglCreateContextAttribsARB_t = HGLRC( WINAPI* )( HDC hdc, HGLRC shareContext, const int* attribs );
							auto  wglCreateContextAttribsARB   = reinterpret_cast< wglCreateContextAttribsARB_t >( wglGetProcAddress( "wglCreateContextAttribsARB" ) );

							if( wglChoosePixelFormatARB && wglCreateContextAttribsARB )
							{
								const int formatAttributes[] =
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

								int  pixelFormats = 0;
								UINT pixelFormatCount = 0;
								wglChoosePixelFormatARB( implGl->deviceContext, formatAttributes, nullptr, 1, &pixelFormats, &pixelFormatCount );

								const int contextAttributes[] =
								{
//									0x2091, 4,          // WGL_CONTEXT_MAJOR_VERSION_ARB = 4
//									0x2092, 0,          // WGL_CONTEXT_MINOR_VERSION_ARB = 0
									0x9126, 0x00000001, // WGL_CONTEXT_PROFILE_MASK_ARB  = WGL_CONTEXT_CORE_PROFILE_BIT_ARB
									0
								};

								implGl->renderContext = wglCreateContextAttribsARB( implGl->deviceContext, nullptr, contextAttributes );
							}
							else
							{
								implGl->renderContext = wglCreateContext( implGl->deviceContext );
							}
						}

						/* Destroy dummy context */
						wglDeleteContext( dummyContext );

						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_X11
					case( window_impl_index_v< __window_impl_x11 > ):
					{
						auto implGl = std::addressof( impl->impl.emplace< __render_context_impl_opengl::__impl_x11 >() );
						implGl->parentWindowImpl = parentWindowImpl;
						implGl->gc               = XCreateGC( implGl->parentWindowImpl->display, implGl->parentWindowImpl->window, 0, nullptr );
						implGl->glxContext       = [ & ]
						{
							/* Create render context */
							{
								int screen = DefaultScreen( implGl->parentWindowImpl->display );
								int attribs[] =
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
									if( !glXQueryVersion( implGl->parentWindowImpl->display, &major, &minor ) )
										break;
									if( ( major < 1 ) || ( major == 1 && minor < 3 ) )
										break;

									using glXCreateContextAttribsARB_t = GLXContext( * )( Display* dpy, GLXFBConfig config, GLXContext shareContext, Bool direct, const int* attribList );
									glXCreateContextAttribsARB_t glXCreateContextAttribsARB = nullptr;
									glXCreateContextAttribsARB = reinterpret_cast< glXCreateContextAttribsARB_t >( glXGetProcAddressARB( reinterpret_cast< const GLubyte* >( "glXCreateContextAttribsARB" ) ) );
									if( !glXCreateContextAttribsARB )
										break;

									int fbConfigCount = 0;
									GLXFBConfig* fbConfigs = glXChooseFBConfig( implGl->parentWindowImpl->display, screen, attribs, &fbConfigCount );
									if( !fbConfigs )
										break;
									if( fbConfigCount == 0 )
										break;

									// Choose the best config
									int bestFbConfigIdx = 0, bestSampleCount = 0;
									for( int i = 0; i < fbConfigCount; ++i )
									{
										XVisualInfo* vi = glXGetVisualFromFBConfig( implGl->parentWindowImpl->display, fbConfigs[ i ] );
										if( vi )
										{
											int samples = 0, sampleCount = 0;
											glXGetFBConfigAttrib( implGl->parentWindowImpl->display, fbConfigs[ i ], GLX_SAMPLE_BUFFERS, &samples );
											glXGetFBConfigAttrib( implGl->parentWindowImpl->display, fbConfigs[ i ], GLX_SAMPLES, &sampleCount );

											if( samples && sampleCount > bestSampleCount )
											{
												bestFbConfigIdx = i;
												bestSampleCount = sampleCount;
											}
										}
										XFree( vi );
									}

									GLXFBConfig bestFbConfig = fbConfigs[ bestFbConfigIdx ];

									XFree( fbConfigs );

									int contextAttribs[] =
									{
//										GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
//										GLX_CONTEXT_MINOR_VERSION_ARB, 0,
										GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
										None
									};

									return glXCreateContextAttribsARB( implGl->parentWindowImpl->display, bestFbConfig, 0, True, contextAttribs );

								} while( false );

								// If all else fails, use legacy method
								auto implX11 = std::get_if< __window_impl_x11 >( implGl->parentWindowImpl );
								XVisualInfo* visualInfo = glXChooseVisual( implX11->display, screen, attribs );
								return glXCreateContext( implX11->display, visualInfo, nullptr, true );
							}
						}();

						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_WAYLAND
					case( window_impl_index_v< __window_impl_wayland > ):
					{
						auto implGl = std::addressof( impl->impl.emplace< __render_context_impl_opengl::__impl_wayland >() );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_COCOA
					case( window_impl_index_v< __window_impl_cocoa > ):
					{
						auto implGl = std::addressof( impl->impl.emplace< __render_context_impl_opengl::__impl_cocoa >() );

						const NSOpenGLPixelFormatAttribute attribs[] =
						{
//							NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
							NSOpenGLPFADoubleBuffer,  true,
							NSOpenGLPFAColorSize,     24,
							NSOpenGLPFADepthSize,     24,
							0
						};

						NSWindow*            nsWindow    = ( NSWindow* )std::get_if< __window_impl_cocoa >( parentWindowImpl )->nsWindow;
						NSOpenGLPixelFormat* pixelFormat = [ NSOpenGLPixelFormat alloc ];
						[ pixelFormat initWithAttributes:attribs ];

						implGl->glView = [ NSOpenGLView alloc ];
						[ ( NSOpenGLView* )glView initWithFrame:nsWindow.contentView.frame pixelFormat:pixelFormat ];
						[ ( NSOpenGLView* )glView prepareOpenGL ];
						[ nsWindow.contentView addSubview:glView ];
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_ANDROID
					case( window_impl_index_v< __window_impl_android > ):
					{
						auto implGl = std::addressof( impl->impl.emplace< __render_context_impl_opengl::__impl_android >() );

						/* Initialize display */
						{
							implGl->eglDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );
							eglInitialize( implGl->eglDisplay, nullptr, nullptr );
						}

						/* Choose config */
						implGl->eglConfig = EGL_NO_CONFIG_KHR;
						do
						{
							EGLint configCount = 0;
							if( !eglGetConfigs( implGl->eglDisplay, nullptr, 0, &configCount ) )
								break;

							std::vector< EGLConfig > configs( static_cast< size_t >( configCount ) );
							if( !eglGetConfigs( implGl->eglDisplay, configs.data(), configs.size(), &configCount ) )
								break;

							const EGLint requiredConformant  = EGL_OPENGL_ES3_BIT_KHR;
							const EGLint requiredSurfaceType = ( EGL_WINDOW_BIT | EGL_PBUFFER_BIT );

							EGLint    bestRedSize    = -1;
							EGLint    bestGreenSize  = -1;
							EGLint    bestBlueSize   = -1;
							EGLint    bestAlphaSize  = -1;
							EGLint    bestBufferSize = -1;
							EGLint    bestDepthSize  = -1;

							for( const EGLConfig& config : configs )
							{
								EGLint conformant = 0;
								eglGetConfigAttrib( implGl->eglDisplay, config, EGL_CONFORMANT, &conformant );
								if( ( conformant & requiredConformant ) == 0 )
									continue;

								EGLint surfaceType = 0;
								eglGetConfigAttrib( implGl->eglDisplay, config, EGL_SURFACE_TYPE, &surfaceType );
								if( ( surfaceType & requiredSurfaceType ) == 0 )
									continue;

								EGLint redSize = 0;
								eglGetConfigAttrib( implGl->eglDisplay, config, EGL_RED_SIZE, &redSize );
								if( redSize < bestRedSize )
									continue;

								EGLint greenSize = 0;
								eglGetConfigAttrib( implGl->eglDisplay, config, EGL_RED_SIZE, &greenSize );
								if( greenSize < bestGreenSize )
									continue;

								EGLint blueSize = 0;
								eglGetConfigAttrib( implGl->eglDisplay, config, EGL_RED_SIZE, &blueSize );
								if( blueSize < bestBlueSize )
									continue;

								EGLint alphaSize = 0;
								eglGetConfigAttrib( implGl->eglDisplay, config, EGL_RED_SIZE, &alphaSize );
								if( alphaSize < bestAlphaSize )
									continue;

								EGLint bufferSize = 0;
								eglGetConfigAttrib( implGl->eglDisplay, config, EGL_BUFFER_SIZE, &bufferSize );
								if( bufferSize < bestBufferSize )
									continue;

								EGLint depthSize = 0;
								eglGetConfigAttrib( implGl->eglDisplay, config, EGL_DEPTH_SIZE, &depthSize );
								if( depthSize < bestDepthSize )
									continue;

								bestRedSize     = redSize;
								bestGreenSize   = greenSize;
								bestBlueSize    = blueSize;
								bestAlphaSize   = alphaSize;
								bestBufferSize  = bufferSize;
								bestDepthSize   = depthSize;
								implGl->eglConfig = config;
							}

							EGLint visualId = 0;
							eglGetConfigAttrib( implGl->eglDisplay, implGl->eglConfig, EGL_NATIVE_VISUAL_ID, &visualId );
							ANativeWindow_setBuffersGeometry( android_only::app->window, 0, 0, visualId );
						} while( false );

						/* Create window surface */
						implGl->eglSurface = eglCreateWindowSurface( implGl->eglDisplay, implGl->eglConfig, android_only::app->window, nullptr );

						/* Create context */
						{
							const EGLint attribs[] =
							{
//								EGL_CONTEXT_CLIENT_VERSION, 3,
								EGL_NONE,
							};

							implGl->eglContext = eglCreateContext( implGl->eglDisplay, implGl->eglConfig, EGL_NO_CONTEXT, attribs );
						}

						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_UIKIT
					case( window_impl_index_v< __window_impl_uikit > ):
					{
						auto implGl = std::addressof( impl->impl.emplace< __render_context_impl_opengl::__impl_uikit >() );

						UIWindow*           uiWindow = ( UIWindow* )std::get_if< __window_impl_uikit >( parentWindowImpl )->uiWindow;
						ORBGLKViewDelegate* delegate = [ ORBGLKViewDelegate alloc ];
						[ delegate init ];

						implGl->eaglContext = [ EAGLContext alloc ];
						[ ( EAGLContext* )impl->eaglContext initWithAPI:kEAGLRenderingAPIOpenGLES3 ];

						implGl->glkView = [ GLKView alloc ];
						[ ( GLKView* )implGl->glkView initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
						( ( GLKView* )implGl->glkView ).context               = ( EAGLContext* )implGl->eaglContext;
						( ( GLKView* )implGl->glkView ).delegate              = delegate;
						( ( GLKView* )implGl->glkView ).enableSetNeedsDisplay = NO;
						[ uiWindow addSubview:( GLKView* )implGl->glkView ];
						break;
					}
				#endif
				}

				/* Load functions */
				make_current();
				impl->functions.emplace();

				const GLubyte* glVersion = glGetString( GL_VERSION );
				impl->embedded = ( glVersion[ 0 ] == u'E' && glVersion[ 1 ] == u'S' );
				if( impl->embedded )
					glVersion += 3;
				impl->version = version( glVersion[ 0 ] - u'0', glVersion[ 2 ] - u'0', glVersion[ 4 ] - u'0' );

				log_info( format( "OpenGL version: %s %d.%d", impl->embedded ? "ES" : "", impl->version.get_major(), impl->version.get_minor() ) );

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case graphics_api::D3D11:
			{
				/* Win32 is the only supported window type for Direct3D 11 */
				if( parentWindow.get_impl_ptr()->index() != window_impl_index_v< __window_impl_win32 > )
					break;

				auto impl             = std::addressof( m_impl.emplace< __render_context_impl_d3d11 >() );
				auto parentWindowImpl = std::get_if< __window_impl_win32 >( parentWindow.get_impl_ptr() );

				/* Find the monitor refresh rate */
				DXGI_RATIONAL refreshRate = { 60000, 1000 };
				{
					com_ptr< IDXGIFactory > factory;
					com_ptr< IDXGIAdapter > adapter;
					com_ptr< IDXGIOutput >  output;
					{ /* Get the primary adapter output. */
						IDXGIObject* tmp;
						CreateDXGIFactory( __uuidof( IDXGIFactory ), reinterpret_cast< void** >( &tmp ) );
						factory.reset( static_cast< IDXGIFactory* >( tmp ) );
						factory->EnumAdapters( 0, reinterpret_cast< IDXGIAdapter** >( &tmp ) );
						adapter.reset( static_cast< IDXGIAdapter* >( tmp ) );
						adapter->EnumOutputs( 0, reinterpret_cast< IDXGIOutput** >( &tmp ) );
						output.reset( static_cast< IDXGIOutput* >( tmp ) );
					}

					UINT monitorWidth, monitorHeight;
					{ /* Get monitor resolution. */
						HMONITOR    monitor = MonitorFromWindow( parentWindowImpl->hwnd, MONITOR_DEFAULTTONEAREST );
						MONITORINFO monitorInfo;
						monitorInfo.cbSize = sizeof( MONITORINFO );
						GetMonitorInfoA( monitor, &monitorInfo );
						monitorWidth       = ( monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left );
						monitorHeight      = ( monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top );
					}

					std::vector< DXGI_MODE_DESC > displayModes;
					{ /* Get display modes. */
						UINT numDisplayModes;
						output->GetDisplayModeList( kBackBufferFormat, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr );
						displayModes.resize( numDisplayModes );
						output->GetDisplayModeList( kBackBufferFormat, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, &displayModes[ 0 ] );
					}

					/* Finally, iterate all the display modes to find one matching the monitor's resolution. */
					for( DXGI_MODE_DESC& mode : displayModes )
					{
						if( mode.Width == monitorWidth &&
							mode.Height == monitorHeight )
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
					constexpr std::array kFeatureLevels =
					{
						D3D_FEATURE_LEVEL_11_1,
						D3D_FEATURE_LEVEL_11_0,
						D3D_FEATURE_LEVEL_10_1,
						D3D_FEATURE_LEVEL_10_0,
						D3D_FEATURE_LEVEL_9_3,
						D3D_FEATURE_LEVEL_9_2,
						D3D_FEATURE_LEVEL_9_1,
					};

					RECT windowRect;
					GetWindowRect( parentWindowImpl->hwnd, &windowRect );

					DXGI_SWAP_CHAIN_DESC desc{ };
					desc.BufferDesc.Width       = ( windowRect.right - windowRect.left );
					desc.BufferDesc.Height      = ( windowRect.bottom - windowRect.top );
					desc.BufferDesc.RefreshRate = refreshRate;
					desc.BufferDesc.Format      = kBackBufferFormat;
					desc.SampleDesc.Count       = 1;
					desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					desc.BufferCount            = 1;
					desc.OutputWindow           = parentWindowImpl->hwnd;
					desc.Windowed               = true;
					desc.SwapEffect             = DXGI_SWAP_EFFECT_DISCARD;

					IDXGISwapChain* swapChain;
					D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, kDeviceFlags, kFeatureLevels.data(), kFeatureLevels.size(), D3D11_SDK_VERSION, &desc, &swapChain, NULL, NULL, NULL );
					impl->swapChain.reset( swapChain );
				}

				/* Get the device */
				{
					void* device;
					impl->swapChain->GetDevice( __uuidof( ID3D11Device ), &device );
					impl->device.reset( static_cast< ID3D11Device* >( device ) );
				}

				/* Get the device context */
				{
					ID3D11DeviceContext* deviceContext;
					impl->device->GetImmediateContext( &deviceContext );
					impl->deviceContext.reset( deviceContext );
				}

				/* Create the render target */
				{
					ID3D11Texture2D* backBuffer;
					impl->swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &backBuffer ) );
					ID3D11RenderTargetView* renderTargetView;
					impl->device->CreateRenderTargetView( backBuffer, NULL, &renderTargetView );
					impl->renderTargetView.reset( renderTargetView );
					backBuffer->Release();
				}

				/* Create the depth stencil */
				{
					RECT rect = { };
					GetWindowRect( parentWindowImpl->hwnd, &rect );

					D3D11_TEXTURE2D_DESC bufferDesc{ };
					bufferDesc.Width            = ( rect.right - rect.left );
					bufferDesc.Height           = ( rect.bottom - rect.top );
					bufferDesc.MipLevels        = 1;
					bufferDesc.ArraySize        = 1;
					bufferDesc.Format           = kDepthBufferFormat;
					bufferDesc.SampleDesc.Count = 1;
					bufferDesc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

					ID3D11Texture2D* depthStencilBuffer;
					impl->device->CreateTexture2D( &bufferDesc, NULL, &depthStencilBuffer );
					impl->depthStencilBuffer.reset( depthStencilBuffer );

					D3D11_DEPTH_STENCIL_DESC stateDesc{ };
					stateDesc.DepthEnable                  = true;
					stateDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
					stateDesc.DepthFunc                    = D3D11_COMPARISON_LESS;
					stateDesc.StencilEnable                = true;
					stateDesc.StencilReadMask              = 0xff;
					stateDesc.StencilWriteMask             = 0xff;
					stateDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
					stateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
					stateDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
					stateDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
					stateDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
					stateDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_DECR;
					stateDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
					stateDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

					ID3D11DepthStencilState* depthStencilState;
					impl->device->CreateDepthStencilState( &stateDesc, &depthStencilState );
					impl->deviceContext->OMSetDepthStencilState( depthStencilState, 1 );
					impl->depthStencilState.reset( depthStencilState );
					
					D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{ };
					viewDesc.Format        = kDepthBufferFormat;
					viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

					ID3D11DepthStencilView* depthStencilView;
					impl->device->CreateDepthStencilView( depthStencilBuffer, &viewDesc, &depthStencilView );

					ID3D11RenderTargetView* renderTargetViews[] = { impl->renderTargetView.get() };
					impl->deviceContext->OMSetRenderTargets( 1, renderTargetViews, depthStencilView );
					impl->depthStencilView.reset( depthStencilView );
				}

				/* Create rasterizer */
				{
					D3D11_RASTERIZER_DESC desc{ };
					desc.CullMode              = D3D11_CULL_BACK;
					desc.DepthClipEnable       = true;
					desc.FillMode              = D3D11_FILL_SOLID;
					desc.FrontCounterClockwise = false;

					ID3D11RasterizerState* rasterizerState;
					impl->device->CreateRasterizerState( &desc, &rasterizerState );
					impl->deviceContext->RSSetState( rasterizerState );
					impl->rasterizerState.reset( rasterizerState );
				}

				/* Set default topology */
				impl->deviceContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

				break;
			}
		#endif
		}

		// Resize context when window is updated
		m_resizeSubscription = parentWindow.subscribe( [ this ]( const window_event& e )
			{
				if( e.type == window_event::Resize )
					this->resize( e.data.resize.w, e.data.resize.h );
			}
		);

		if( CurrentContext == nullptr )
		{
			make_current();
		}
	}

	render_context::~render_context()
	{
		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				[[ maybe_unused ]] auto impl = std::get_if< __render_context_impl_opengl >( &m_impl );

				switch( impl->impl.index() )
				{
					default: break;

				#if __ORB_HAS_WINDOW_API_WIN32
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_win32 > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_win32 >( &impl->impl );
						if( CurrentContext == this )
							wglMakeCurrent( implGl->deviceContext, NULL );
						wglDeleteContext( implGl->renderContext );
						ReleaseDC( std::get_if< __window_impl_win32 >( implGl->parentWindowImpl )->hwnd, implGl->deviceContext );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_X11
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_x11 > ):
					{
						auto implGl  = std::get_if< __render_context_impl_opengl::__impl_x11 >( &impl->impl );
						auto implX11 = std::get_if< __window_impl_x11 >( implGl->parentWindowImpl );
						if( CurrentContext == this )
							glXMakeCurrent( implX11->display, None, nullptr );
						glXDestroyContext( implX11->display, implGl->glxContext );
						XFreeGC( implX11->display, implGl->gc );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_WAYLAND
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_wayland > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_wayland >( &impl->impl );
						if( CurrentContext == this )
							do { } while( false );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_COCOA
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_cocoa > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_cocoa >( &impl->impl );
						if( CurrentContext == this )
							[ NSOpenGLContext clearCurrentContext ];
						[ ( const NSOpenGLView* )implGl->glView removeFromSuperview ];
						[ ( const NSOpenGLView* )implGl->glView dealloc ];
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_ANDROID
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_android > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_android >( &impl->impl );
						if( CurrentContext == this )
							eglMakeCurrent( implGl->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
						eglDestroyContext( implGl->eglDisplay, implGl->eglContext );
						eglDestroySurface( implGl->eglDisplay, implGl->eglSurface );
						eglTerminate( implGl->eglDisplay );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_UIKIT
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_uikit > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_uikit >( &impl->impl );
						if( CurrentContext == this )
							[ EAGLContext setCurrentContext:nullptr ];
						[ ( GLKView* )implGl->glkView dealloc ];
						[ ( EAGLContext* )implGl->eaglContext dealloc ];
						break;
					}
				#endif
				}

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
//			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
//			{
//				break;
//			}
		#endif
		}

		if( CurrentContext == this )
		{
			CurrentContext = nullptr;
		}
	}

	bool render_context::make_current()
	{
	#if __ORB_HAS_GRAPHICS_API_OPENGL
		if( m_impl.index() == render_context_impl_index_v< __render_context_impl_opengl > )
		{
			auto impl = std::get_if< __render_context_impl_opengl >( &m_impl );
			switch( impl->impl.index() )
			{
				default: break;

			#if __ORB_HAS_WINDOW_API_WIN32
				case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_win32 > ):
				{
					auto implGl = std::get_if< __render_context_impl_opengl::__impl_win32 >( &impl->impl );
					if( !wglMakeCurrent( implGl->deviceContext, implGl->renderContext ) )
						return false;
					break;
				}
			#endif

			#if __ORB_HAS_WINDOW_API_X11
				case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_x11 > ):
				{
					auto implGl  = std::get_if< __render_context_impl_opengl::__impl_x11 >( &impl->impl );
					auto implX11 = std::get_if< __window_impl_x11 >( implGl->parentWindowImpl );
					if( !glXMakeCurrent( implX11->display, implX11->window, implGl->glxContext ) )
						return false;
					break;
				}
			#endif

			#if __ORB_HAS_WINDOW_API_WAYLAND
//				case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_wayland > ):
//				{
//					break;
//				}
			#endif

			#if __ORB_HAS_WINDOW_API_COCOA
				case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_cocoa > ):
				{
					auto implGl = std::get_if< __render_context_impl_opengl::__impl_cocoa >( &impl->impl );
					[ [ ( const NSOpenGLView* )implGl->glView openGLContext ] makeCurrentContext ];
					break;
				}
			#endif

			#if __ORB_HAS_WINDOW_API_ANDROID
				case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_android > ):
				{
					auto implGl = std::get_if< __render_context_impl_opengl::__impl_android >( &impl->impl );
					eglMakeCurrent( implGl->eglDisplay, implGl->eglSurface, implGl->eglSurface, implGl->eglContext );
					break;
				}
			#endif

			#if __ORB_HAS_WINDOW_API_UIKIT
				case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_uikit > ):
				{
					auto implGl = std::get_if< __render_context_impl_opengl::__impl_uikit >( &impl->impl );
					[ EAGLContext setCurrentContext:( EAGLContext* )implGl->eaglContext ];
					break;
				}
			#endif
			}
		}
	#endif

		CurrentContext = this;

		return true;
	}

	void render_context::resize( uint32_t width, uint32_t height )
	{
		render_context* prev_current_context = CurrentContext;
		make_current();

		switch( m_impl.index() )
		{
			default:
			{
				( void )width;
				( void )height;
				break;
			}

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				auto impl = std::get_if< __render_context_impl_opengl >( &m_impl );
				switch( impl->impl.index() )
				{
					default:
					case( opengl_render_context_impl_index_v< std::monostate > ):
						break;

				#if __ORB_HAS_WINDOW_API_ANDROID
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_android > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_android >( &impl->impl );
						eglMakeCurrent( implGl->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
						if( implGl->eglSurface != EGL_NO_SURFACE )
							eglDestroySurface( implGl->eglDisplay, implGl->eglSurface );
						implGl->eglSurface = eglCreateWindowSurface( implGl->eglDisplay, implGl->eglConfig, android_only::app->window, nullptr );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_UIKIT
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_uikit > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_uikit >( &impl->impl );
						( ( GLKView* )implGl->glkView ).layer.frame = CGRectMake( 0.f, 0.f, width, height );
						break;
					}
				#endif
				}

				glViewport( 0, 0, width, height );

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
			{
				auto impl = std::get_if< __render_context_impl_d3d11 >( &m_impl );

				impl->deviceContext->OMSetRenderTargets( 0, nullptr, nullptr );
				impl->deviceContext->ClearState();
				impl->deviceContext->Flush();

				D3D11_TEXTURE2D_DESC depthStencilBufferDesc{ };
				impl->depthStencilBuffer->GetDesc( &depthStencilBufferDesc );

				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{ };
				impl->depthStencilView->GetDesc( &depthStencilViewDesc );

				impl->renderTargetView.reset();
				impl->depthStencilBuffer.reset();
				impl->depthStencilView.reset();

				DXGI_SWAP_CHAIN_DESC swapChainDesc{ };
				impl->swapChain->GetDesc( &swapChainDesc );
				swapChainDesc.BufferDesc.Width  = width;
				swapChainDesc.BufferDesc.Height = height;
				impl->swapChain->ResizeBuffers( 1, swapChainDesc.BufferDesc.Width, swapChainDesc.BufferDesc.Height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags );

				/* Recreate render target */
				{
					ID3D11Texture2D* backBuffer;
					impl->swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &backBuffer ) );
					ID3D11RenderTargetView* renderTargetView;
					impl->device->CreateRenderTargetView( backBuffer, nullptr, &renderTargetView );
					impl->renderTargetView.reset( renderTargetView );
					backBuffer->Release();
				}

				/* Recreate depth stencil */
				{
					depthStencilBufferDesc.Width  = width;
					depthStencilBufferDesc.Height = height;

					ID3D11Texture2D* depthStencilBuffer;
					impl->device->CreateTexture2D( &depthStencilBufferDesc, NULL, &depthStencilBuffer );
					impl->depthStencilBuffer.reset( depthStencilBuffer );

					ID3D11DepthStencilView* depthStencilView;
					impl->device->CreateDepthStencilView( depthStencilBuffer, &depthStencilViewDesc, &depthStencilView );
					impl->depthStencilView.reset( depthStencilView );
				}

				ID3D11RenderTargetView* renderTargetViews[] = { impl->renderTargetView.get() };
				impl->deviceContext->OMSetRenderTargets( 1, renderTargetViews, impl->depthStencilView.get() );
				impl->deviceContext->OMSetDepthStencilState( impl->depthStencilState.get(), 0 );
				impl->deviceContext->RSSetState( impl->rasterizerState.get() );
				impl->deviceContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

				D3D11_VIEWPORT viewport{ };
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;
				viewport.Width    = static_cast< FLOAT >( width );
				viewport.Height   = static_cast< FLOAT >( height );
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				impl->deviceContext->RSSetViewports( 1, &viewport );

				RECT scissor{ };
				scissor.left   = 0;
				scissor.right  = width;
				scissor.top    = 0;
				scissor.bottom = height;
				impl->deviceContext->RSSetScissorRects( 1, &scissor );

				break;
			}
		#endif
		}

		if( prev_current_context != this )
			prev_current_context->make_current();
	}

	void render_context::swap_buffers()
	{
		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				auto impl = std::get_if< __render_context_impl_opengl >( &m_impl );

				switch( impl->impl.index() )
				{
					default: break;

				#if __ORB_HAS_WINDOW_API_WIN32
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_win32 > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_win32 >( &impl->impl );
						SwapBuffers( implGl->deviceContext );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_X11
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_x11 > ):
					{
						auto implGl  = std::get_if< __render_context_impl_opengl::__impl_x11 >( &impl->impl );
						auto implX11 = std::get_if< __window_impl_x11 >( implGl->parentWindowImpl );
						glXSwapBuffers( implX11->display, implX11->window );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_WAYLAND
//					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_wayland > ):
//					{
//						break;
//					}
				#endif

				#if __ORB_HAS_WINDOW_API_COCOA
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_cocoa > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_cocoa >( &impl->impl );
						[ [ ( const NSOpenGLView* )implGl->glView openGLContext ] flushBuffer ];
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_ANDROID
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_android > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_android >( &impl->impl );
						eglSwapBuffers( implGl->eglDisplay, implGl->eglSurface );
						break;
					}
				#endif

				#if __ORB_HAS_WINDOW_API_UIKIT
					case( opengl_render_context_impl_index_v< __render_context_impl_opengl::__impl_uikit > ):
					{
						auto implGl = std::get_if< __render_context_impl_opengl::__impl_uikit >( &impl->impl );
						[ ( GLKView* )implGl->glkView display ];
						break;
					}
				#endif
				}

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
			{
				auto impl = std::get_if< __render_context_impl_d3d11 >( &m_impl );
				impl->swapChain->Present( 0, 0 );
				break;
			}
		#endif
		}
	}

	void render_context::clear( buffer_mask mask )
	{
		switch( m_impl.index() )
		{
			default:
			{
				( void )mask;
				break;
			}

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				GLbitfield glmask = 0;
				glmask |= ( ( !!( mask & buffer_mask::Color ) ) ? GL_COLOR_BUFFER_BIT : 0 );
				glmask |= ( ( !!( mask & buffer_mask::Depth ) ) ? GL_DEPTH_BUFFER_BIT : 0 );
				glClear( glmask );
				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
			{
				auto impl = std::get_if< __render_context_impl_d3d11 >( &m_impl );
				if( !!( mask & buffer_mask::Color ) )
					impl->deviceContext->ClearRenderTargetView( impl->renderTargetView.get(), &impl->clearColor[ 0 ] );
				if( !!( mask & buffer_mask::Depth ) )
					impl->deviceContext->ClearDepthStencilView( impl->depthStencilView.get(), D3D11_CLEAR_DEPTH, 1.0f, 0 );
				break;
			}
		#endif
		}
	}

	void render_context::set_clear_color( float r, float g, float b )
	{
		switch( m_impl.index() )
		{
			default:
			{
				( void )r;
				( void )g;
				( void )b;
				break;
			}

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				glClearColor( r, g, b, 1.0f );
				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
			{
				auto impl = std::get_if< __render_context_impl_d3d11 >( &m_impl );
				impl->clearColor.r = r;
				impl->clearColor.g = g;
				impl->clearColor.b = b;
				impl->clearColor.a = 1.0f;
				break;
			}
		#endif
		}
	}

	render_context* render_context::get_current()
	{
		return CurrentContext;
	}
}

#if __ORB_HAS_WINDOW_API_UIKIT
@implementation ORBGLKViewDelegate

-( void )glkView:( nonnull GLKView* )view drawInRect:( CGRect )rect
{
	/* Unused parameters */
	( void )view;
	( void )rect;
}

@end
#endif
