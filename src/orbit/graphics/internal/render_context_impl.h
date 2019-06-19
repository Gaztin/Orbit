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

#include "orbit/core/internal/window_impl.h"
#include "orbit/core/color.h"
#include "orbit/graphics/platform/opengl/gl.h"
#include "orbit/graphics.h"

/* Assume that OpenGL is always available */
#define __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL 1

#if __has_include( <d3d11.h> )
#  define __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11 1
#else
#  define __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11 0
#endif

#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
#  include <d3d11.h>
#endif

namespace orb
{

#define __ORB_NUM_RENDER_CONTEXT_IMPLS ( __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL + \
                                         __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11 )

	enum class render_context_impl_type
	{
		Null = 0,
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
		OpenGL,
	#endif
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
		D3D11,
	#endif
	};

	union render_context_impl_storage
	{
		render_context_impl_storage()
			: null{ }
		{
		}

		struct
		{
		} null;
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
		union opengl_t
		{
			opengl_t() : null{ } { }

			window_impl_type               parentWindowImplType;
			std::optional< gl::functions > functions;

			struct
			{
			} null;
		#if __ORB_HAS_WINDOW_IMPL_WIN32
			struct
			{
				window_impl_storage* parentWindowImpl;
				HDC                  deviceContext;
				HGLRC                renderContext;
			} wgl;
		#elif __ORB_HAS_WINDOW_IMPL_X11
			struct
			{
				window_impl_storage* parentWindowImpl;
				GC                   gc;
				GLXContext           glxContext;
			} glx;
		#elif __ORB_HAS_WINDOW_IMPL_WAYLAND
			struct
			{
			} wl;
		#elif __ORB_HAS_WINDOW_IMPL_COCOA
			struct
			{
				void* glView;
			} cocoa;
		#elif __ORB_HAS_WINDOW_IMPL_ANDROID
			struct
			{
				EGLDisplay eglDisplay;
				EGLConfig  eglConfig;
				EGLSurface eglSurface;
				EGLContext eglContext;
			} egl;
		#elif __ORB_HAS_WINDOW_IMPL_UIKIT
			struct
			{
				void* eaglContext;
				void* glkView;
			} glkit;
		#endif
		} opengl;
	#endif

	#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
		struct
		{
//			com_ptr< IDXGISwapChain >          swapChain;
//			com_ptr< ID3D11Device >            device;
//			com_ptr< ID3D11DeviceContext >     deviceContext;
//			com_ptr< ID3D11RenderTargetView >  renderTargetView;
//			com_ptr< ID3D11Texture2D >         depthStencilBuffer;
//			com_ptr< ID3D11DepthStencilState > depthStencilState;
//			com_ptr< ID3D11DepthStencilView >  depthStencilView;
//			com_ptr< ID3D11RasterizerState >   rasterizerState;
//			color                              clearColor;
			IDXGISwapChain*          swapChain;
			ID3D11Device*            device;
			ID3D11DeviceContext*     deviceContext;
			ID3D11RenderTargetView*  renderTargetView;
			ID3D11Texture2D*         depthStencilBuffer;
			ID3D11DepthStencilState* depthStencilState;
			ID3D11DepthStencilView*  depthStencilView;
			ID3D11RasterizerState*   rasterizerState;
			color                    clearColor;
		} d3d11;
	#endif
	};

	constexpr render_context_impl_type DefaultRenderContextImpl =
	#if __ORB_HAS_RENDER_CONTEXT_IMPL_D3D11
		render_context_impl_type::D3D11;
	#elif __ORB_HAS_RENDER_CONTEXT_IMPL_OPENGL
		render_context_impl_type::OpenGL;
	#else
		render_context_impl_type::Null;
	#endif
}
