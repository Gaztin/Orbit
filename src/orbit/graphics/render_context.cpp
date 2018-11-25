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

#include "render_context.h"

#include "orbit/core/window.h"
#include "platform/d3d11/graphics_platform_d3d11.h"
#include "platform/opengl/graphics_platform_gl.h"

namespace orb
{

static platform::render_context_handle init_handle(const platform::window_handle& wh, graphics_api api)
{
	switch (api)
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL: return platform::gl::create_render_context_handle(wh);
#endif
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11: return platform::d3d11::create_render_context_handle(wh);
#endif
		default: return *cast<platform::render_context_handle*>(nullptr);
	}
}

render_context::render_context(window& parentWindow, graphics_api api)
	: m_api(api)
	, m_parentWindowHandle(parentWindow.get_handle())
	, m_handle(init_handle(m_parentWindowHandle, api))
{
#if defined(ORB_HAS_OPENGL)
	if (m_api == graphics_api::OpenGL)
		platform::gl::make_current(m_handle);
#endif

	// Resize context when window is updated
	m_resizeSubscription = parentWindow.subscribe(
		[this](const window_event& e)
	{
		if (e.type == window_event::Resize)
			this->resize(e.data.resize.w, e.data.resize.h);
	});
}

render_context::~render_context()
{
	switch (m_api)
	{
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			m_handle.d3d11.rasterizerState->Release();
			m_handle.d3d11.depthStencilView->Release();
			m_handle.d3d11.depthStencilState->Release();
			m_handle.d3d11.depthStencilBuffer->Release();
			m_handle.d3d11.renderTargetView->Release();
			m_handle.d3d11.deviceContext->Release();
			m_handle.d3d11.device->Release();
			m_handle.d3d11.swapChain->Release();
			break;
#endif

#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			platform::gl::destroy_context_handle(m_parentWindowHandle, m_handle);
			break;
#endif

		default:
			break;
	}
}


void render_context::resize(uint32_t width, uint32_t height)
{
	switch (m_api)
	{
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			platform::d3d11::flush_device_context(m_handle);
			platform::d3d11::resize_swap_chain(m_handle, width, height);
			platform::d3d11::recreate_buffers(m_handle, m_parentWindowHandle);
			break;
#endif

#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			platform::gl::recreate_surface(m_handle, width, height);
			platform::gl::make_current(m_handle);
			glViewport(0, 0, width, height);
			break;
#endif

		default:
			break;
	}
}

void render_context::swap_buffers()
{
	switch (m_api)
	{
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			platform::d3d11::present(m_handle);
			break;
#endif

#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			platform::gl::swap_buffers(m_handle);
			break;
#endif

		default:
			break;
	}
}

void render_context::clear(buffer_mask mask)
{
	switch (m_api)
	{
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			if (!!(mask & buffer_mask::Color))
				platform::d3d11::clear_render_target(m_handle);
			if (!!(mask & buffer_mask::Depth))
				platform::d3d11::clear_depth_stencil(m_handle);
			break;
#endif

#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			glClear(
				(!!(mask & buffer_mask::Color)) ? GL_COLOR_BUFFER_BIT : 0 |
				(!!(mask & buffer_mask::Depth)) ? GL_DEPTH_BUFFER_BIT : 0);
			break;
#endif

		default:
			break;
	}
}

void render_context::set_clear_color(float r, float g, float b)
{
	switch (m_api)
	{
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			m_handle.d3d11.clearColor.r = r;
			m_handle.d3d11.clearColor.g = g;
			m_handle.d3d11.clearColor.b = b;
			break;
#endif

#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			glClearColor(r, g, b, 1.0f);
			break;
#endif

		default:
			break;
	}
}

}
