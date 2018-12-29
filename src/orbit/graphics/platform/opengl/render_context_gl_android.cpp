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

#include "orbit/core/android_app.h"

namespace orb
{
namespace platform
{

static EGLDisplay init_display()
{
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, nullptr, nullptr);
	return display;
}

static EGLConfig choose_config(const EGLDisplay& display)
{
	constexpr EGLint ConfigAttribs[] =
	{
		EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_RED_SIZE,        8,
		EGL_GREEN_SIZE,      8,
		EGL_BLUE_SIZE,       8,
		EGL_ALPHA_SIZE,      8,
		EGL_DEPTH_SIZE,      24,
		EGL_NONE
	};
	EGLConfig config;
	EGLint numConfigs;
	eglChooseConfig(display, ConfigAttribs, &config, 1, &numConfigs);

	EGLint visualId;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &visualId);
	ANativeWindow_setBuffersGeometry(android_only::app->window, 0, 0, visualId);
	return config;
}

static EGLSurface create_surface(const EGLDisplay& display, const EGLConfig& config)
{
	return eglCreateWindowSurface(display, config, android_only::app->window, nullptr);
}

static EGLContext create_context(const EGLDisplay& display, const EGLConfig& config)
{
	constexpr EGLint Attribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,
	};

	return eglCreateContext(display, config, EGL_NO_CONTEXT, Attribs);
}

render_context_gl::render_context_gl(const window_handle& wh)
	: m_eglDisplay(init_display())
	, m_eglConfig(choose_config(m_eglDisplay))
	, m_eglSurface(create_surface(m_eglDisplay, m_eglConfig))
	, m_eglContext(create_context(m_eglDisplay, m_eglConfig))
{
	make_current();
	m_functions = gl::load_functions();
	make_current(nullptr);
}

render_context_gl::~render_context_gl()
{
	eglDestroyContext(m_eglDisplay, m_eglContext);
	eglDestroySurface(m_eglDisplay, m_eglSurface);
	eglTerminate(m_eglDisplay);
}

bool render_context_gl::make_current()
{
	return (eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext) != 0);
}

bool render_context_gl::make_current(std::nullptr_t)
{
	return (eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != 0);
}

void render_context_gl::resize(uint32_t /*width*/, uint32_t /*height*/)
{
	eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	if (m_eglSurface != EGL_NO_SURFACE)
		eglDestroySurface(m_eglDisplay, m_eglSurface);

	m_eglSurface = create_surface(m_eglDisplay, m_eglConfig);
}

void render_context_gl::swap_buffers()
{
	eglSwapBuffers(m_eglDisplay, m_eglSurface);
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
