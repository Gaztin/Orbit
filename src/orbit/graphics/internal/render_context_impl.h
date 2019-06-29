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

#pragma once

#include <optional>
#include <type_traits>
#include <variant>

#include "orbit/core/internal/window_impl.h"
#include "orbit/core/memory.h"
#include "orbit/core/color.h"
#include "orbit/core/version.h"
#include "orbit/graphics/internal/graphics_api.h"
#include "orbit/graphics/platform/opengl/gl.h"
#include "orbit/graphics.h"

namespace orb
{
#if __ORB_HAS_GRAPHICS_API_OPENGL
	struct __render_context_impl_opengl
	{
		version version;

	#if __ORB_HAS_WINDOW_API_WIN32
		struct __impl_win32
		{
			window_impl* parentWindowImpl;
			HDC          deviceContext;
			HGLRC        renderContext;
		};
	#endif
	#if __ORB_HAS_WINDOW_API_X11
		struct __impl_x11
		{
			window_impl_storage* parentWindowImpl;
			GC                   gc;
			GLXContext           glxContext;
		};
	#endif
	#if __ORB_HAS_WINDOW_API_WAYLAND
		struct __impl_wayland
		{
		};
	#endif
	#if __ORB_HAS_WINDOW_API_COCOA
		struct __impl_cocoa
		{
			void* glView;
		};
	#endif
	#if __ORB_HAS_WINDOW_API_ANDROID
		struct __impl_android
		{
			EGLDisplay eglDisplay;
			EGLConfig  eglConfig;
			EGLSurface eglSurface;
			EGLContext eglContext;
		};
	#endif
	#if __ORB_HAS_WINDOW_API_UIKIT
		struct __impl_uikit
		{
			void* eaglContext;
			void* glkView;
		};
	#endif

		using __impl = std::variant< std::monostate
	#if __ORB_HAS_WINDOW_API_WIN32
		, __impl_win32
	#endif
	#if __ORB_HAS_WINDOW_API_X11
		, __impl_x11
	#endif
	#if __ORB_HAS_WINDOW_API_WAYLAND
		, __impl_wayland
	#endif
	#if __ORB_HAS_WINDOW_API_COCOA
		, __impl_cocoa
	#endif
	#if __ORB_HAS_WINDOW_API_ANDROID
		, __impl_android
	#endif
	#if __ORB_HAS_WINDOW_API_UIKIT
		, __impl_uikit
	#endif
		>;

		std::optional< gl::functions > functions;
		__impl                         impl;
	};
#endif

#if __ORB_HAS_GRAPHICS_API_D3D11
	struct __render_context_impl_d3d11
	{
		com_ptr< IDXGISwapChain >          swapChain;
		com_ptr< ID3D11Device >            device;
		com_ptr< ID3D11DeviceContext >     deviceContext;
		com_ptr< ID3D11RenderTargetView >  renderTargetView;
		com_ptr< ID3D11Texture2D >         depthStencilBuffer;
		com_ptr< ID3D11DepthStencilState > depthStencilState;
		com_ptr< ID3D11DepthStencilView >  depthStencilView;
		com_ptr< ID3D11RasterizerState >   rasterizerState;
		color                              clearColor;
	};
#endif

	using render_context_impl = std::variant< std::monostate
#if __ORB_HAS_GRAPHICS_API_OPENGL
		, __render_context_impl_opengl
#endif
#if __ORB_HAS_GRAPHICS_API_D3D11
		, __render_context_impl_d3d11
#endif
	>;

}
