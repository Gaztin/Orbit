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

#pragma once
#include "Orbit/Core/Platform/Windows/ComPtr.h"
#include "Orbit/Core/Private/WindowDetails.h"
#include "Orbit/Core/Utility/Color.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLVersion.h"
#include "Orbit/Graphics/API/OpenGL/OpenGL.h"
#include "Orbit/Graphics/Renderer/BlendEquation.h"

#include <map>
#include <optional>
#include <type_traits>
#include <variant>

#if defined( ORB_OS_MACOS )
@class NSOpenGLView;
#elif defined( ORB_OS_IOS ) // ORB_OS_MACOS
@class EAGLContext;
@class GLKView;
#endif // ORB_OS_IOS

ORB_NAMESPACE_BEGIN

namespace Private
{

#if( ORB_HAS_OPENGL )

	struct _RenderContextDetailsOpenGL
	{
		OpenGLVersion version;

	#if defined( ORB_OS_WINDOWS )

		HDC   hdc;
		HGLRC hglrc;

	#elif defined( ORB_OS_LINUX ) // ORB_OS_WINDOWS

		GC         gc;
		GLXContext context;

	#elif defined( ORB_OS_MACOS ) // ORB_OS_LINUX

		NSOpenGLView* view;

	#elif defined( ORB_OS_ANDROID ) // ORB_OS_MACOS

		EGLDisplay display;
		EGLConfig  config;
		EGLSurface surface;
		EGLContext context;

	#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

		EAGLContext* context;
		GLKView*     view;

	#endif // ORB_OS_IOS

	};

#endif // ORB_HAS_OPENGL
#if( ORB_HAS_D3D11 )

	struct _RenderContextDetailsD3D11
	{
		ComPtr< IDXGISwapChain1 >         swap_chain;
		ComPtr< ID3D11Device >            device;
		ComPtr< ID3D11DeviceContext >     device_context;
		ComPtr< ID3D11RenderTargetView >  render_target_view;
		ComPtr< ID3D11Texture2D >         depth_stencil_buffer;
		ComPtr< ID3D11DepthStencilState > depth_stencil_state;
		ComPtr< ID3D11DepthStencilView >  depth_stencil_view;
		ComPtr< ID3D11RasterizerState >   rasterizer_state;
		ComPtr< ID3D11BlendState >        disable_blending_blend_state;

		std::map< BlendEquation, ComPtr< ID3D11BlendState > > blend_states;

		Color clear_color;
	};

#endif // ORB_HAS_D3D11

	using RenderContextDetails = std::variant< std::monostate
	#if( ORB_HAS_OPENGL )
		, _RenderContextDetailsOpenGL
	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )
		, _RenderContextDetailsD3D11
	#endif // ORB_HAS_D3D11
	>;

}

ORB_NAMESPACE_END
