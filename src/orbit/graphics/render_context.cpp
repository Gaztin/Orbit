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

#include <assert.h>

#include "orbit/graphics/internal/render_context_d3d11_impl.h"
#include "orbit/graphics/internal/render_context_opengl_impl.h"
#include "orbit/core/events/window_event.h"
#include "orbit/core/internal/window_impl.h"
#include "orbit/core/window.h"
#include "orbit/core/utility.h"

namespace orb
{

static GLbitfield buffer_bits(uint32_t mask)
{
	GLbitfield bitfield = 0;
	bitfield |= (mask & buffer_mask::Color) ? GL_COLOR_BUFFER_BIT : 0;
	bitfield |= (mask & buffer_mask::Depth) ? GL_DEPTH_BUFFER_BIT : 0;
	return bitfield;
}

static void recreate_surface(window& parentWindow, render_context& rc, graphics_api api)
{
	switch (api)
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			rc.ref<render_context_opengl_impl>().recreate_surface(parentWindow.ref<window_impl>());
			break;
#endif
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			rc.ref<render_context_d3d11_impl>().recreate_swap_chain(parentWindow.ref<window_impl>());
			break;
#endif

		default:
			assert(false);
	}
}

render_context::render_context(window& parentWindow, graphics_api api)
	: m_api(api)
	, m_windowResizeSubscription{}
{
	switch (m_api)
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			construct<render_context_opengl_impl>(parentWindow.ref<window_impl>());
			break;
#endif
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			construct<render_context_d3d11_impl>(parentWindow.ref<window_impl>());
			break;
#endif
		default:
			assert(false);
	}

	m_windowResizeSubscription = parentWindow.subscribe(
		[this, &parentWindow](const window_event& e)
	{
		switch (e.type)
		{
			case window_event::Restore:
			case window_event::Resize:
				recreate_surface(parentWindow, *this, m_api);
				break;

			default:
				break;
		}
	});
}

render_context::~render_context()
{
	window::unsubscribe(m_windowResizeSubscription);
}

void render_context::make_current(const window& parentWindow)
{
	switch (m_api)
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			ref<render_context_opengl_impl>().make_current(parentWindow.ref<window_impl>());
			break;
#endif
		default:
			break;
	}
}

void render_context::swap_buffers(const window& parentWindow)
{
	switch (m_api)
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			ref<render_context_opengl_impl>().swap_buffers(parentWindow.ref<window_impl>());
			break;
#endif
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			ref<render_context_d3d11_impl>().swap_buffers();
			break;
#endif
		default:
			assert(false);
	}
}

void render_context::clear(uint32_t mask)
{
	switch (m_api)
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			assert(ref<render_context_opengl_impl>().is_current());
			glClear(buffer_bits(mask));
			break;
#endif
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			ref<render_context_d3d11_impl>().clear(mask);
			break;
#endif
		default:
			assert(false);
	}
}

void render_context::set_clear_color(float r, float g, float b)
{
	switch (m_api)
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			assert(ref<render_context_opengl_impl>().is_current());
			glClearColor(r, g, b, 1.0f);
			break;
#endif
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			ref<render_context_d3d11_impl>().set_clear_color(r, g, b, 1.0f);
			break;
#endif
		default:
			assert(false);
	}
}

}
