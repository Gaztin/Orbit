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
#include "orbit/graphics/platform/opengl/gl_version.h"

namespace orb
{
namespace platform
{

static GC create_gc(Display* display, const Window& window)
{
	return XCreateGC(display, window, 0, nullptr);
}

static GLXContext create_glx_context(Display* display, gl::version v)
{
	int screen = DefaultScreen(display);
	int attribs[] =
	{
		GLX_DOUBLEBUFFER,
		GLX_RGBA,
		GLX_DEPTH_SIZE, 24,
		None
	};

	// TODO: Initialize using provided OpenGL version
	(void)v;

	XVisualInfo* visualInfo = glXChooseVisual(display, screen, attribs);
	return glXCreateContext(display, visualInfo, nullptr, true);
}

render_context_gl::render_context_gl(const window_handle& wh)
	: m_wndPtr(&wh)
	, m_gc(create_gc(wh.display, wh.window))
	, m_glxContext(create_glx_context(wh.display))
{
	make_current();
	m_functions = gl::load_functions();
	make_current(nullptr);
}

render_context_gl::~render_context_gl()
{
	glXDestroyContext(m_wndPtr->display, m_glxContext);
	XFreeGC(m_wndPtr->display, m_gc);
}

bool render_context_gl::make_current()
{
	return glXMakeCurrent(m_wndPtr->display, m_wndPtr->window, m_glxContext);
}

bool render_context_gl::make_current(std::nullptr_t)
{
	return glXMakeCurrent(m_wndPtr->display, None, nullptr);
}

void render_context_gl::resize(uint32_t width, uint32_t height)
{
	glViewport(0, 0, width, height);
}

void render_context_gl::swap_buffers()
{
	glXSwapBuffers(m_wndPtr->display, m_wndPtr->window);
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
