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

#include "render_context_gl.h"

#include "orbit/core/platform/window_handle.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/platform/opengl/gl_version.h"

namespace orb
{
namespace platform
{

static void set_pixel_format(HDC hdc)
{
	PIXELFORMATDESCRIPTOR desc{};
	desc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	desc.nVersion = 1;
	desc.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	desc.iPixelType = PFD_TYPE_RGBA;
	desc.cColorBits = 24;
	desc.cDepthBits = 24;
	desc.iLayerType = PFD_MAIN_PLANE;

	const int format = ChoosePixelFormat(hdc, &desc);
	SetPixelFormat(hdc, format, &desc);
}

render_context_gl::render_context_gl(const window_handle& wh, gl::version v)
	: m_parentHwnd(wh.hwnd)
	, m_hdc(GetDC(m_parentHwnd))
{
	set_pixel_format(m_hdc);

	// TODO: Initialize using provided OpenGL version
	(void)v;

	m_hglrc = wglCreateContext(m_hdc);
	
	make_current();
	m_functions = gl::load_functions();
	make_current(nullptr);
}

render_context_gl::~render_context_gl()
{
	wglDeleteContext(m_hglrc);
	ReleaseDC(m_parentHwnd, m_hdc);
}

bool render_context_gl::make_current()
{
	return wglMakeCurrent(m_hdc, m_hglrc);
}

bool render_context_gl::make_current(std::nullptr_t)
{
	return wglMakeCurrent(m_hdc, nullptr);
}

void render_context_gl::resize(uint32_t width, uint32_t height)
{
	glViewport(0, 0, width, height);
}

void render_context_gl::swap_buffers()
{
	SwapBuffers(m_hdc);
}

void render_context_gl::set_clear_color(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
}

void render_context_gl::clear_buffers(buffer_mask mask)
{
	glClear(
		(!!(mask & buffer_mask::Color)) ? GL_COLOR_BUFFER_BIT : 0 |
		(!!(mask & buffer_mask::Depth)) ? GL_DEPTH_BUFFER_BIT : 0);
}

}
}
