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

#include "orbit/core/log.h"
#include "orbit/core/window.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/platform/d3d11/render_context_d3d11.h"
#include "orbit/graphics/platform/opengl/render_context_gl.h"

#if( __ORB_NUM_RENDER_CONTEXT_IMPLS > 1 )
#  define RENDER_CONTEXT_IMPL_SWITCH switch( m_implType )
#  define RENDER_CONTEXT_IMPL_CASE( TYPE ) case TYPE:
#  define RENDER_CONTEXT_IMPL_BREAK break
#  define RENDER_CONTEXT_IMPL_IF( TYPE ) if( m_implType == TYPE )
#else
#  define RENDER_CONTEXT_IMPL_SWITCH
#  define RENDER_CONTEXT_IMPL_CASE( TYPE )
#  define RENDER_CONTEXT_IMPL_BREAK
#  define RENDER_CONTEXT_IMPL_IF( TYPE )
#endif

#if( __ORB_NUM_WINDOW_IMPLS > 1 )
#  define WINDOW_IMPL_SWITCH( VAR ) switch( ( VAR ) )
#  define WINDOW_IMPL_CASE( TYPE ) case ( TYPE ):
#  define WINDOW_IMPL_BREAK break
#else
#  define WINDOW_IMPL_SWITCH( VAR )
#  define WINDOW_IMPL_CASE( TYPE )
#  define WINDOW_IMPL_BREAK
#endif

namespace orb
{
#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
	constexpr DXGI_FORMAT kBackBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr DXGI_FORMAT kDepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
#endif

	static render_context* CurrentContext = nullptr;

	render_context::render_context( window& parentWindow, render_context_impl_type type )
		: m_storage{ }
		, m_resizeSubscription{ }
	#if ( __ORB_NUM_RENDER_CONTEXT_IMPLS > 1 )
		, m_implType( type )
	#endif
	{
		RENDER_CONTEXT_IMPL_SWITCH
		{
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::OpenGL )
			{
				WINDOW_IMPL_SWITCH( m_storage.opengl.parentWindowImplType = parentWindow.get_impl_type() )
				{
				#if __ORB_HAS_WINDOW_IMPL_WIN32
					WINDOW_IMPL_CASE( window_impl_type::Win32 )
					{
						auto impl = &( m_storage.opengl.wgl = { } );
						impl->parentWindowImpl = parentWindow.get_impl_ptr();

						/* Set pixel format */
						{
							impl->deviceContext = GetDC( impl->parentWindowImpl->win32.hwnd );

							PIXELFORMATDESCRIPTOR desc{ };
							desc.nSize      = sizeof( PIXELFORMATDESCRIPTOR );
							desc.nVersion   = 1;
							desc.dwFlags    = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
							desc.iPixelType = PFD_TYPE_RGBA;
							desc.cColorBits = 24;
							desc.cDepthBits = 32;
							desc.iLayerType = PFD_MAIN_PLANE;
							const int format = ChoosePixelFormat( impl->deviceContext, &desc );

							SetPixelFormat( impl->deviceContext, format, &desc );
						}

						/* Create dummy context */
						HGLRC dummyContext = wglCreateContext( impl->deviceContext );
						wglMakeCurrent( impl->deviceContext, dummyContext );

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
								wglChoosePixelFormatARB( impl->deviceContext, formatAttributes, nullptr, 1, &pixelFormats, &pixelFormatCount );

								const int contextAttributes[] =
								{
//									0x2091, 4,          // WGL_CONTEXT_MAJOR_VERSION_ARB = 4
//									0x2092, 0,          // WGL_CONTEXT_MINOR_VERSION_ARB = 0
									0x9126, 0x00000001, // WGL_CONTEXT_PROFILE_MASK_ARB  = WGL_CONTEXT_CORE_PROFILE_BIT_ARB
									0
								};

								wglCreateContextAttribsARB( impl->deviceContext, nullptr, contextAttributes );
							}
							else
							{
								wglCreateContext( impl->deviceContext );
							}
						}

						/* Destroy dummy context */
						wglDeleteContext( dummyContext );

						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_X11
					WINDOW_IMPL_CASE( window_impl_type::X11 )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_WAYLAND
					WINDOW_IMPL_CASE( window_impl_type::Wayland )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_COCOA
					WINDOW_IMPL_CASE( window_impl_type::Cocoa )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_ANDROID
					WINDOW_IMPL_CASE( window_impl_type::Android )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_UIKIT
					WINDOW_IMPL_CASE( window_impl_type::UiKit )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				}

				/* Load functions */
				make_current();
				//m_storage.opengl.functions = std::make_optional< gl::functions >();
				m_storage.opengl.functions.emplace();
				log_info( format( "GL_VERSION: %s", reinterpret_cast< const char* >( glGetString( GL_VERSION ) ) ) );

				RENDER_CONTEXT_IMPL_BREAK;
			}
	#endif
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::D3D11 )
			{
				auto impl = &( m_storage.d3d11 = { } );

				if( parentWindow.get_impl_type() != window_impl_type::Win32 )
					break;
				auto parentWindowImpl = parentWindow.get_impl_ptr();

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
						HMONITOR    monitor = MonitorFromWindow( parentWindowImpl->win32.hwnd, MONITOR_DEFAULTTONEAREST );
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
					constexpr UINT deviceFlags = 0
					#if !defined( NDEBUG )
						| D3D11_CREATE_DEVICE_DEBUG
					#endif
						;
					constexpr std::array featureLevels =
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
					GetWindowRect( parentWindowImpl->win32.hwnd, &windowRect );

					DXGI_SWAP_CHAIN_DESC desc{ };
					desc.BufferDesc.Width       = ( windowRect.right - windowRect.left );
					desc.BufferDesc.Height      = ( windowRect.bottom - windowRect.top );
					desc.BufferDesc.RefreshRate = refreshRate;
					desc.BufferDesc.Format      = kBackBufferFormat;
					desc.SampleDesc.Count       = 1;
					desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					desc.BufferCount            = 1;
					desc.OutputWindow           = parentWindowImpl->win32.hwnd;
					desc.Windowed               = true;
					desc.SwapEffect             = DXGI_SWAP_EFFECT_DISCARD;

					D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, featureLevels.data(), featureLevels.size(), D3D11_SDK_VERSION, &desc, &impl->swapChain, NULL, NULL, NULL );
				}

				/* Get the device */
				impl->swapChain->GetDevice( __uuidof( ID3D11Device ), reinterpret_cast< void** >( &impl->device ) );

				/* Get the device context */
				impl->device->GetImmediateContext( &impl->deviceContext );

				/* Create the render target */
				{
					ID3D11Texture2D* backBuffer;
					impl->swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &backBuffer ) );
					impl->device->CreateRenderTargetView( backBuffer, NULL, &impl->renderTargetView );
					backBuffer->Release();
				}

				/* Create the depth stencil */
				{
					RECT rect = { };
					GetWindowRect( parentWindowImpl->win32.hwnd, &rect );

					D3D11_TEXTURE2D_DESC bufferDesc{ };
					bufferDesc.Width            = ( rect.right - rect.left );
					bufferDesc.Height           = ( rect.bottom - rect.top );
					bufferDesc.MipLevels        = 1;
					bufferDesc.ArraySize        = 1;
					bufferDesc.Format           = kDepthBufferFormat;
					bufferDesc.SampleDesc.Count = 1;
					bufferDesc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

					impl->device->CreateTexture2D( &bufferDesc, NULL, &impl->depthStencilBuffer );

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

					impl->device->CreateDepthStencilState( &stateDesc, &impl->depthStencilState );
					impl->deviceContext->OMSetDepthStencilState( impl->depthStencilState, 1 );
					
					D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{ };
					viewDesc.Format        = kDepthBufferFormat;
					viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

					impl->device->CreateDepthStencilView( impl->depthStencilBuffer, &viewDesc, &impl->depthStencilView );
					impl->deviceContext->OMSetRenderTargets( 1, &impl->renderTargetView, impl->depthStencilView );
				}

				/* Create rasterizer */
				{
					D3D11_RASTERIZER_DESC desc{ };
					desc.CullMode              = D3D11_CULL_BACK;
					desc.DepthClipEnable       = true;
					desc.FillMode              = D3D11_FILL_SOLID;
					desc.FrontCounterClockwise = false;
					impl->device->CreateRasterizerState( &desc, &impl->rasterizerState );
					impl->deviceContext->RSSetState( impl->rasterizerState );
				}

				/* Set default topology */
				impl->deviceContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

				RENDER_CONTEXT_IMPL_BREAK;
			}
	#endif
		}

		// Resize context when window is updated
		m_resizeSubscription = parentWindow.subscribe(
			[ this ]( const window_event& e )
			{
				if( e.type == window_event::Resize )
					this->resize( e.data.resize.w, e.data.resize.h );
			}
		);
	}

	render_context::~render_context()
	{
		RENDER_CONTEXT_IMPL_SWITCH
		{
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::OpenGL )
			{
				[[ maybe_unused ]] auto impl = &m_storage.opengl;

				WINDOW_IMPL_SWITCH( impl->parentWindowImplType )
				{
				#if __ORB_HAS_WINDOW_IMPL_WIN32
					WINDOW_IMPL_CASE( window_impl_type::Win32 )
					{
						auto wgl = &m_storage.opengl.wgl;
						if( CurrentContext == this )
							wglMakeCurrent( wgl->deviceContext, NULL );
						wglDeleteContext( wgl->renderContext );
						ReleaseDC( wgl->parentWindowImpl->win32.hwnd, wgl->deviceContext );
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_X11
					WINDOW_IMPL_CASE( window_impl_type::X11 )
					{
						auto impl = &m_storage.opengl.glx;
						if( CurrentContext == this )
							do { } while( false );
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_WAYLAND
					WINDOW_IMPL_CASE( window_impl_type::Wayland )
					{
						auto impl = &m_storage.opengl.wl;
						if( CurrentContext == this )
							do { } while( false );
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_COCOA
					WINDOW_IMPL_CASE( window_impl_type::Cocoa )
					{
						auto impl = &m_storage.opengl.cocoa;
						if( CurrentContext == this )
							do { } while( false );
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_ANDROID
					WINDOW_IMPL_CASE( window_impl_type::Android )
					{
						auto impl = &m_storage.opengl.egl;
						if( CurrentContext == this )
							do { } while( false );
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_UIKIT
					WINDOW_IMPL_CASE( window_impl_type::UiKit )
					{
						auto impl = &m_storage.opengl.glkit;
						if( CurrentContext == this )
							do { } while( false );
						WINDOW_IMPL_BREAK;
					}
				#endif
				}

				RENDER_CONTEXT_IMPL_BREAK;
			}
	#endif
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::D3D11 )
			{
				RENDER_CONTEXT_IMPL_BREAK;
			}
	#endif
		}

		if( CurrentContext == this )
		{
			CurrentContext = nullptr;
		}
	}

	bool render_context::make_current()
	{
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
		RENDER_CONTEXT_IMPL_IF( render_context_impl_type::OpenGL )
		{
			WINDOW_IMPL_SWITCH( m_storage.opengl.parentWindowImplType )
			{
			#if __ORB_HAS_WINDOW_IMPL_WIN32
				WINDOW_IMPL_CASE( window_impl_type::Win32 )
				{
					auto impl = &m_storage.opengl.wgl;
					if( !wglMakeCurrent( impl->deviceContext, impl->renderContext ) )
						return false;
					WINDOW_IMPL_BREAK;
				}
			#endif
			#if __ORB_HAS_WINDOW_IMPL_X11
				WINDOW_IMPL_CASE( window_impl_type::X11 )
				{
					WINDOW_IMPL_BREAK;
				}
			#endif
			#if __ORB_HAS_WINDOW_IMPL_WAYLAND
				WINDOW_IMPL_CASE( window_impl_type::Wayland )
				{
					WINDOW_IMPL_BREAK;
				}
			#endif
			#if __ORB_HAS_WINDOW_IMPL_COCOA
				WINDOW_IMPL_CASE( window_impl_type::Cocoa )
				{
					WINDOW_IMPL_BREAK;
				}
			#endif
			#if __ORB_HAS_WINDOW_IMPL_ANDROID
				WINDOW_IMPL_CASE( window_impl_type::Android )
				{
					WINDOW_IMPL_BREAK;
				}
			#endif
			#if __ORB_HAS_WINDOW_IMPL_UIKIT
				WINDOW_IMPL_CASE( window_impl_type::UiKit )
				{
					WINDOW_IMPL_BREAK;
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

		RENDER_CONTEXT_IMPL_SWITCH
		{
		#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::OpenGL )
			{
				glViewport( 0, 0, width, height );
				RENDER_CONTEXT_IMPL_BREAK;
			}
		#endif
		#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::D3D11 )
			{
				auto impl = &m_storage.d3d11;

				impl->deviceContext->OMSetRenderTargets( 0, nullptr, nullptr );
				impl->deviceContext->ClearState();
				impl->deviceContext->Flush();

				DXGI_SWAP_CHAIN_DESC swapChainDesc{ };
				impl->swapChain->GetDesc( &swapChainDesc );
				swapChainDesc.BufferDesc.Width  = width;
				swapChainDesc.BufferDesc.Height = height;
				impl->swapChain->ResizeBuffers( 1, swapChainDesc.BufferDesc.Width, swapChainDesc.BufferDesc.Height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags );

				/* Recreate render target */
				{
					impl->renderTargetView->Release();

					ID3D11Texture2D* backBuffer;
					impl->swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &backBuffer ) );
					impl->device->CreateRenderTargetView( backBuffer, nullptr, &impl->renderTargetView );
					backBuffer->Release();
				}

				/* Recreate depth stencil */
				{
					D3D11_TEXTURE2D_DESC bufferDesc{ };
					impl->depthStencilBuffer->GetDesc( &bufferDesc );
					bufferDesc.Width  = width;
					bufferDesc.Height = height;

					impl->depthStencilBuffer->Release();
					impl->device->CreateTexture2D( &bufferDesc, NULL, &impl->depthStencilBuffer );

					D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{ };
					impl->depthStencilView->GetDesc( &viewDesc );

					impl->depthStencilView->Release();
					impl->device->CreateDepthStencilView( impl->depthStencilBuffer, &viewDesc, &impl->depthStencilView );
					impl->deviceContext->OMSetRenderTargets( 1, &impl->renderTargetView, impl->depthStencilView );
				}

				impl->deviceContext->OMSetRenderTargets( 1, &impl->renderTargetView, impl->depthStencilView );
				impl->deviceContext->OMSetDepthStencilState( impl->depthStencilState, 0 );
				impl->deviceContext->RSSetState( impl->rasterizerState );
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

				RENDER_CONTEXT_IMPL_BREAK;
			}
		#endif
		}

		if( prev_current_context != this )
			prev_current_context->make_current();
	}

	void render_context::swap_buffers()
	{
		RENDER_CONTEXT_IMPL_SWITCH
		{
		#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::OpenGL )
			{
				auto impl = &m_storage.opengl;

				WINDOW_IMPL_SWITCH( impl->parentWindowImplType )
				{
				#if __ORB_HAS_WINDOW_IMPL_WIN32
					WINDOW_IMPL_CASE( window_impl_type::Win32 )
					{
						SwapBuffers( impl->wgl.deviceContext );
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_X11
					WINDOW_IMPL_CASE( window_impl_type::X11 )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_WAYLAND
					WINDOW_IMPL_CASE( window_impl_type::Wayland )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_COCOA
					WINDOW_IMPL_CASE( window_impl_type::Cocoa )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_ANDROID
					WINDOW_IMPL_CASE( window_impl_type::Android )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				#if __ORB_HAS_WINDOW_IMPL_UIKIT
					WINDOW_IMPL_CASE( window_impl_type::UiKit )
					{
						WINDOW_IMPL_BREAK;
					}
				#endif
				}

				RENDER_CONTEXT_IMPL_BREAK;
			}
		#endif
		#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::D3D11 )
			{
				m_storage.d3d11.swapChain->Present( 0, 0 );
				RENDER_CONTEXT_IMPL_BREAK;
			}
		#endif
		}
	}

	void render_context::clear( buffer_mask mask )
	{
		RENDER_CONTEXT_IMPL_SWITCH
		{
		#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::OpenGL )
			{
				GLbitfield glmask = 0;
				glmask |= ( ( !!( mask & buffer_mask::Color ) ) ? GL_COLOR_BUFFER_BIT : 0 );
				glmask |= ( ( !!( mask & buffer_mask::Depth ) ) ? GL_DEPTH_BUFFER_BIT : 0 );
				glClear( glmask );
				RENDER_CONTEXT_IMPL_BREAK;
			}
		#endif
		#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::D3D11 )
			{
				auto impl = &m_storage.d3d11;
				if( !!( mask & buffer_mask::Color ) )
					impl->deviceContext->ClearRenderTargetView( impl->renderTargetView, &impl->clearColor[ 0 ] );
				if( !!( mask & buffer_mask::Depth ) )
					impl->deviceContext->ClearDepthStencilView( impl->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
				RENDER_CONTEXT_IMPL_BREAK;
			}
		#endif
		}
	}

	void render_context::set_clear_color( float r, float g, float b )
	{
		RENDER_CONTEXT_IMPL_SWITCH
		{
		#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::OpenGL )
			{
				glClearColor( r, g, b, 1.0f );
				RENDER_CONTEXT_IMPL_BREAK;
			}
		#endif
		#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
			RENDER_CONTEXT_IMPL_CASE( render_context_impl_type::D3D11 )
			{
				auto impl = &m_storage.d3d11;
				impl->clearColor.r = r;
				impl->clearColor.g = g;
				impl->clearColor.b = b;
				impl->clearColor.a = 1.0f;
				RENDER_CONTEXT_IMPL_BREAK;
			}
		#endif
		}
	}

	render_context* render_context::get_current()
	{
		return CurrentContext;
	}
}
