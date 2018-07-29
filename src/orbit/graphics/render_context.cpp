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
#include "orbit/core/internal/window_impl.h"
#include "orbit/core/window.h"
#include "orbit/core/utility.h"

namespace orb
{

using opengl = render_context_opengl_impl;
using d3d11 = render_context_d3d11_impl;

static GLbitfield buffer_bits(uint32_t mask)
{
	GLbitfield bitfield = 0;
	bitfield |= (mask & buffer_mask::Color & ~0) ? GL_COLOR_BUFFER_BIT : 0;
	bitfield |= (mask & buffer_mask::Depth & ~0) ? GL_DEPTH_BUFFER_BIT : 0;
	return bitfield;
}

render_context::render_context(const window& parentWindow, graphics_api api)
	: m_api(api)
{
	switch (m_api)
	{
		case graphics_api::OpenGL:
			construct<opengl>(parentWindow.impl<window_impl>());
			break;

		case graphics_api::D3D11:
			construct<d3d11>(parentWindow.impl<window_impl>());
			break;

		default:
			assert(false);
	}
}

render_context::~render_context()
{
	switch (m_api)
	{
		case graphics_api::OpenGL:
			destruct<opengl>();
			break;

		case graphics_api::D3D11:
			destruct<d3d11>();
			break;

		default:
			assert(false);
	}
}

void render_context::make_current(const window& parentWindow)
{
	switch (m_api)
	{
		case graphics_api::OpenGL:
			impl<opengl>().make_current(parentWindow.impl<window_impl>());
			break;

		default:
			break;
	}
}

void render_context::swap_buffers(const window& parentWindow)
{
	switch (m_api)
	{
		case graphics_api::OpenGL:
			impl<opengl>().swap_buffers(parentWindow.impl<window_impl>());
			break;

		case graphics_api::D3D11:
			impl<d3d11>().swap_buffers();
			break;

		default: assert(false);
	}
}

void render_context::clear(uint32_t mask)
{
	switch (m_api)
	{
		case graphics_api::OpenGL:
			assert(impl<opengl>().is_current());
			glClear(buffer_bits(mask));
			break;

		case graphics_api::D3D11:
			impl<d3d11>().clear(mask);
			break;

		default: assert(false);
	}
}

void render_context::set_clear_color(float r, float g, float b)
{
	switch (m_api)
	{
		case graphics_api::OpenGL:
			assert(impl<opengl>().is_current());
			glClearColor(r, g, b, 1.0f);
			break;

		case graphics_api::D3D11:
			impl<d3d11>().set_clear_color(r, g, b, 1.0f);
			break;

		default: assert(false);
	}
}

}
