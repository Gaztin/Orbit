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

#include "orbit/graphics/api/d3d11/context_d3d11.h"
#include "orbit/graphics/api/opengl/context_gl.h"
#include "orbit/core/window.h"

namespace orb
{

render_context::render_context(window& parentWindow, graphics_api api)
	: m_api(api)
{
	switch (m_api)
	{
#if defined(ORB_HAS_D3D11)
		case graphics_api::D3D11:
			m_context = std::make_unique<d3d11::context>(parentWindow.get_handle());
			break;
#endif
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL:
			m_context = std::make_unique<gl::context>(parentWindow.get_handle());
			break;
#endif
		default:
			throw;
	}

	// Resize context when window is updated
	m_resizeSubscription = parentWindow.subscribe(
		[this](const window_event& e)
	{
		if (e.type == window_event::Resize)
			this->resize(e.data.resize.w, e.data.resize.h);
	});
}

void render_context::resize(uint32_t width, uint32_t height)
{
	m_context->resize(width, height);
}

void render_context::swap_buffers()
{
	m_context->swap_buffers();
}

void render_context::clear(buffer_mask bm)
{
	m_context->clear(bm);
}

void render_context::set_clear_color(float r, float g, float b)
{
	m_context->set_clear_color(r, g, b);
}

}
