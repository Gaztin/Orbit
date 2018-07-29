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

#include <android/native_window.h>
#include <orbit/core/android_app.h>

#include "orbit/core/android_app.h"

namespace orb
{

render_context_opengl_impl::render_context_opengl_impl(const window_impl& parentWindowImpl)
	: m_display(create_display())
	, m_surface(create_surface())
	, m_context(create_context())
{
}

render_context_opengl_impl::~render_context_opengl_impl()
{
	eglDestroyContext(m_display, m_context);
	eglDestroySurface(m_display, m_surface);
	eglTerminate(m_display);
}

void render_context_opengl_impl::make_current(const window_impl&)
{
	eglMakeCurrent(m_display, m_surface, m_surface, m_context);
}

void render_context_opengl_impl::swap_buffers(const window_impl&)
{
	eglSwapBuffers(m_display, m_surface);
}

void render_context_opengl_impl::reset_current()
{
	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

bool render_context_opengl_impl::is_current() const
{
	return (eglGetCurrentContext() == m_context);
}

EGLDisplay render_context_opengl_impl::create_display() const
{
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, nullptr, nullptr);
	return display;
}

EGLSurface render_context_opengl_impl::create_surface() const
{
	constexpr EGLint configAttribs[] =
	{
		EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
		EGL_RED_SIZE,        8,
		EGL_GREEN_SIZE,      8,
		EGL_BLUE_SIZE,       8,
		EGL_ALPHA_SIZE,      8,
		EGL_DEPTH_SIZE,      24,
		EGL_NONE
	};
	EGLConfig config;
	EGLint numConfigs;
	eglChooseConfig(m_display, configAttribs, &config, 1, &numConfigs);

	EGLint visualId;
	eglGetConfigAttrib(m_display, config, EGL_NATIVE_VISUAL_ID, &visualId);
	ANativeWindow_setBuffersGeometry(android_only::app->window, 0, 0, visualId);
	return eglCreateWindowSurface(m_display, config, android_only::app->window, nullptr);
}

EGLContext render_context_opengl_impl::create_context() const
{
	constexpr EGLint attribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 1,
		EGL_NONE,
	};
	EGLConfig config;
	EGLint numConfig;
	eglGetConfigs(m_display, &config, 1, &numConfig);
	return eglCreateContext(m_display, config, EGL_NO_CONTEXT, attribs);
}

}
