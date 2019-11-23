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

#pragma once
#include <optional>
#include <type_traits>
#include <variant>

#include "Orbit/Core/Internal/WindowImpl.h"
#include "Orbit/Core/Memory.h"
#include "Orbit/Core/Color.h"
#include "Orbit/Core/Version.h"
#include "Orbit/Graphics/Internal/GraphicsAPI.h"
#include "Orbit/Graphics/Platform/OpenGL/OpenGL.h"
#include "Orbit/Graphics.h"

ORB_NAMESPACE_BEGIN

#if _ORB_HAS_GRAPHICS_API_OPENGL
struct _RenderContextImplOpenGL
{
	bool    embedded;
	Version opengl_version;

#if _ORB_HAS_WINDOW_API_WIN32
	struct _SubImplWin32
	{
		WindowImpl* parent_window_impl;
		HDC         hdc;
		HGLRC       hglrc;
	};
#endif
#if _ORB_HAS_WINDOW_API_X11
	struct _SubImplX11
	{
		WindowImpl* parent_window_impl;
		GC          gc;
		GLXContext  context;
	};
#endif
#if _ORB_HAS_WINDOW_API_WAYLAND
	struct _SubImplWayland
	{
	};
#endif
#if _ORB_HAS_WINDOW_API_COCOA
	struct _SubImplCocoa
	{
		void* view;
	};
#endif
#if _ORB_HAS_WINDOW_API_ANDROID
	struct _SubImplAndroid
	{
		EGLDisplay display;
		EGLConfig  config;
		EGLSurface surface;
		EGLContext context;
	};
#endif
#if _ORB_HAS_WINDOW_API_UIKIT
	struct _SubImplUIKit
	{
		void* context;
		void* view;
	};
#endif

	using _SubImpl = std::variant< std::monostate
#if _ORB_HAS_WINDOW_API_WIN32
	, _SubImplWin32
#endif
#if _ORB_HAS_WINDOW_API_X11
	, _SubImplX11
#endif
#if _ORB_HAS_WINDOW_API_WAYLAND
	, _SubImplWayland
#endif
#if _ORB_HAS_WINDOW_API_COCOA
	, _SubImplCocoa
#endif
#if _ORB_HAS_WINDOW_API_ANDROID
	, _SubImplAndroid
#endif
#if _ORB_HAS_WINDOW_API_UIKIT
	, _SubImplUIKit
#endif
	>;

	std::optional< OpenGL::Functions > functions;
	_SubImpl                           sub_impl;
};
#endif

#if _ORB_HAS_GRAPHICS_API_D3D11
struct _RenderContextImplD3D11
{
	ComPtr< IDXGISwapChain >          swap_chain;
	ComPtr< ID3D11Device >            device;
	ComPtr< ID3D11DeviceContext >     device_context;
	ComPtr< ID3D11RenderTargetView >  render_target_view;
	ComPtr< ID3D11Texture2D >         depth_stencil_buffer;
	ComPtr< ID3D11DepthStencilState > depth_stencil_state;
	ComPtr< ID3D11DepthStencilView >  depth_stencil_view;
	ComPtr< ID3D11RasterizerState >   rasterizer_state;
	Color                             clear_color;
};
#endif

using RenderContextImpl = std::variant< std::monostate
#if _ORB_HAS_GRAPHICS_API_OPENGL
	, _RenderContextImplOpenGL
#endif
#if _ORB_HAS_GRAPHICS_API_D3D11
	, _RenderContextImplD3D11
#endif
>;

ORB_NAMESPACE_END
