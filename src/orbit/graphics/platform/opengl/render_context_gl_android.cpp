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

#include <vector>

#include "orbit/core/android_app.h"
#include "orbit/graphics/platform/opengl/gl_version.h"

namespace orb
{
namespace platform
{

static EGLint get_renderable_type(gl::version v)
{
	switch (v)
	{
		case gl::version::vES_2: return EGL_OPENGL_ES2_BIT;
		case gl::version::vES_3: return EGL_OPENGL_ES3_BIT_KHR;
		default:                 return 0;
	}
}

static EGLint get_context_client(gl::version v)
{
	switch (v)
	{
		case gl::version::vES_2: return 2;
		case gl::version::vES_3: return 3;
		default:                 return 0;
	}
}

static EGLDisplay init_display()
{
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, nullptr, nullptr);
	return display;
}

static EGLConfig choose_config(const EGLDisplay& display, gl::version v)
{
	EGLint configCount = 0;
	if (!eglGetConfigs(display, nullptr, 0, &configCount))
		return EGL_NO_CONFIG_KHR;

	std::vector<EGLConfig> configs(static_cast<size_t>(configCount));
	if (!eglGetConfigs(display, configs.data(), configs.size(), &configCount))
		return EGL_NO_CONFIG_KHR;

	const EGLint requiredConformant  = get_renderable_type(v);
	const EGLint requiredSurfaceType = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;

	EGLConfig bestConfig     = EGL_NO_CONFIG_KHR;
	EGLint    bestRedSize    = -1;
	EGLint    bestGreenSize  = -1;
	EGLint    bestBlueSize   = -1;
	EGLint    bestAlphaSize  = -1;
	EGLint    bestBufferSize = -1;
	EGLint    bestDepthSize  = -1;

	for (const EGLConfig& config : configs)
	{
		EGLint conformant  = 0;
		eglGetConfigAttrib(display, config, EGL_CONFORMANT,   &conformant);
		if ((conformant & requiredConformant) == 0)
			continue;

		EGLint surfaceType = 0;
		eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);
		if ((surfaceType & requiredSurfaceType) == 0)
			continue;

		EGLint redSize = 0;
		eglGetConfigAttrib(display, config, EGL_RED_SIZE, &redSize);
		if (redSize < bestRedSize)
			continue;

		EGLint greenSize = 0;
		eglGetConfigAttrib(display, config, EGL_RED_SIZE, &greenSize);
		if (greenSize < bestGreenSize)
			continue;

		EGLint blueSize = 0;
		eglGetConfigAttrib(display, config, EGL_RED_SIZE, &blueSize);
		if (blueSize < bestBlueSize)
			continue;

		EGLint alphaSize = 0;
		eglGetConfigAttrib(display, config, EGL_RED_SIZE, &alphaSize);
		if (alphaSize < bestAlphaSize)
			continue;

		EGLint bufferSize = 0;
		eglGetConfigAttrib(display, config, EGL_BUFFER_SIZE, &bufferSize);
		if (bufferSize < bestBufferSize)
			continue;

		EGLint depthSize = 0;
		eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depthSize);
		if (depthSize < bestDepthSize)
			continue;

		bestConfig     = config;
		bestRedSize    = redSize;
		bestGreenSize  = greenSize;
		bestBlueSize   = blueSize;
		bestAlphaSize  = alphaSize;
		bestBufferSize = bufferSize;
		bestDepthSize  = depthSize;
	}

	EGLint visualId = 0;
	eglGetConfigAttrib(display, bestConfig, EGL_NATIVE_VISUAL_ID, &visualId);
	ANativeWindow_setBuffersGeometry(android_only::app->window, 0, 0, visualId);

	return bestConfig;
}

static EGLSurface create_surface(const EGLDisplay& display, const EGLConfig& config)
{
	return eglCreateWindowSurface(display, config, android_only::app->window, nullptr);
}

static EGLContext create_context(const EGLDisplay& display, const EGLConfig& config, gl::version v)
{
	const EGLint attribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, get_context_client(v),
		EGL_NONE,
	};

	return eglCreateContext(display, config, EGL_NO_CONTEXT, attribs);
}

render_context_gl::render_context_gl(const window_handle& wh, gl::version v)
	: m_eglDisplay(init_display())
	, m_eglConfig(choose_config(m_eglDisplay, v))
	, m_eglSurface(create_surface(m_eglDisplay, m_eglConfig))
	, m_eglContext(create_context(m_eglDisplay, m_eglConfig, v))
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
