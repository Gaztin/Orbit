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

#include "context_gl.h"

#include "orbit/graphics/platform/opengl/graphics_platform_gl.h"

namespace orb
{
namespace gl
{

context::context(const orb::platform::window_handle& wh)
	: m_parentWindowHandle(wh)
	, m_handle(orb::platform::gl::create_context_handle(wh))
{
	// TODO: Figure out an object-oriented approach to manage "current" contexts
	orb::platform::gl::make_current(m_handle);
}

context::~context()
{
	orb::platform::gl::destroy_context_handle(m_parentWindowHandle, m_handle);
}

void context::resize(uint32_t width, uint32_t height)
{
	orb::platform::gl::recreate_surface(m_handle, width, height);
	orb::platform::gl::make_current(m_handle);
	glViewport(0, 0, width, height);
}

void context::swap_buffers()
{
	orb::platform::gl::swap_buffers(m_handle);
}

void context::clear(buffer_mask mask)
{
	glClear(orb::platform::gl::get_buffer_bits(mask));
}

void context::set_clear_color(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
}

}
}
