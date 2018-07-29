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

#include "render_context_opengl_impl.h"

#include <assert.h>

#include "orbit/core/internal/window_impl.h"

namespace orb
{

render_context_opengl_impl::render_context_opengl_impl(const window_impl& parentWindowImpl)
	: m_display(parentWindowImpl.display())
	, m_gc(XCreateGC(parentWindowImpl.display(), parentWindowImpl.window(), 0, nullptr))
	, m_context(create_glx_context(parentWindowImpl.display()))
{
}

render_context_opengl_impl::~render_context_opengl_impl()
{
	glXDestroyContext(m_display, m_context);
	XFreeGC(m_display, m_gc);
}

void render_context_opengl_impl::make_current(const window_impl& parentWindowImpl)
{
	assert(m_display == parentWindowImpl.display());
	glXMakeCurrent(m_display, parentWindowImpl.window(), m_context);
}

void render_context_opengl_impl::swap_buffers(const window_impl& parentWindowImpl)
{
	assert(m_display == parentWindowImpl.display());
	glXSwapBuffers(m_display, parentWindowImpl.window());
}

void render_context_opengl_impl::reset_current()
{
	glXMakeCurrent(m_display, None, nullptr);
}

bool render_context_opengl_impl::is_current() const
{
	return (glXGetCurrentContext() == m_context);
}

GLXContext render_context_opengl_impl::create_glx_context(Display* display)
{
	int screen = DefaultScreen(display);
	int attribs[] =
	{
		GLX_DOUBLEBUFFER,
		GLX_RGBA,
		GLX_DEPTH_SIZE, 24,
		None
	};
	XVisualInfo* visualInfo = glXChooseVisual(display, screen, attribs);
	GLXContext context = glXCreateContext(display, visualInfo, nullptr, true);
	return context;
}

}

