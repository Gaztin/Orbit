/*
* Copyright (c) 2018 Sebastian Kylander http://gaztin.com/
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
#include "orbit/graphics.h"

#include <memory>

#include <d3d11.h>
#include <dxgi.h>

#include "orbit/core/color.h"
#include "orbit/core/utility.h"

namespace orb
{
namespace platform
{

union render_context_handle
{
	struct d3d11_t
	{
		IDXGISwapChain* swapChain;
		ID3D11Device* device;
		ID3D11DeviceContext* deviceContext;
		ID3D11RenderTargetView* renderTargetView;
		ID3D11Texture2D* depthStencilBuffer;
		ID3D11DepthStencilState* depthStencilState;
		ID3D11DepthStencilView* depthStencilView;
		ID3D11RasterizerState* rasterizerState;

		color clearColor;

	} d3d11;

	struct gl_t
	{
#if defined(ORB_OS_WINDOWS)
		HDC hdc;
		HGLRC hglrc;
#elif defined(ORB_OS_LINUX)
		const window_handle* wndPtr;
		GC gc;
		GLXContext glxContext;
#elif defined(ORB_OS_MACOS)
		void* glView; // <GLView*>
#elif defined(ORB_OS_ANDROID)
		EGLDisplay eglDisplay;
		EGLConfig eglConfig;
		EGLSurface eglSurface;
		EGLContext eglContext;
#elif defined(ORB_OS_IOS)
		void* eaglContext; // <EAGLContext*>
		void* glkView; // <GLKView*>
#endif
	} gl;

	render_context_handle(in_place_type<d3d11_t>) : d3d11{} { }
	render_context_handle(in_place_type<gl_t>)    : gl   {} { }
};

}
}
