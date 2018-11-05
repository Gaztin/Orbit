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

#include <gl/GL.h>

namespace orb
{
namespace platform
{

static GLbitfield convert_buffer_mask(buffer_mask mask)
{
	GLbitfield bits = 0;
	bits |= (!!(mask & buffer_mask::Color)) ? GL_COLOR_BUFFER_BIT : 0;
	bits |= (!!(mask & buffer_mask::Depth)) ? GL_DEPTH_BUFFER_BIT : 0;
	return bits;
}

context_gl::context_gl(const window_handle& wh)
	: m_parentWindowHandle(wh)
	, m_handle{}
//	, m_handle(platform::gl::create_context(wh))
{
	m_handle.hdc = GetDC(wh.hwnd);
	m_handle.hglrc = wglCreateContext(m_handle.hdc);

	// TODO: Figure out an object-oriented approach to manage "current" contexts
	wglMakeCurrent(m_handle.hdc, m_handle.hglrc);
}

context_gl::~context_gl()
{
	ReleaseDC(m_parentWindowHandle.hwnd, m_handle.hdc);
}

void context_gl::resize(uint32_t width, uint32_t height)
{
	glViewport(0, 0, width, height);
}

void context_gl::swap_buffers()
{
	SwapBuffers(m_handle.hdc);
}

void context_gl::clear(buffer_mask mask)
{
	glClear(convert_buffer_mask(mask));
}

void context_gl::set_clear_color(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
}

}
}
