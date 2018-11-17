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

#include "graphics_platform_gl.h"

#include <android/native_window.h>

#include "orbit/core/platform/window_handle.h"
#include "orbit/core/android_app.h"

namespace orb
{
namespace platform
{
namespace gl
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
		EGL_CONTEXT_CLIENT_VERSION, 1,
		EGL_NONE,
	};

	/*EGLConfig config;
	EGLint numConfig;
	eglGetConfigs(display, &config, 1, &numConfig);*/
	return eglCreateContext(display, config, EGL_NO_CONTEXT, Attribs);
}

context_handle create_context_handle(const window_handle& wh)
{
	context_handle ch{};
	ch.eglDisplay = init_display();
	ch.eglConfig = choose_config(ch.eglDisplay);
	ch.eglSurface = create_surface(ch.eglDisplay, ch.eglConfig);
	ch.eglContext = create_context(ch.eglDisplay, ch.eglConfig);
	return ch;
}

void destroy_context_handle(const window_handle& /*wh*/, const context_handle& ch)
{
	eglDestroyContext(ch.eglDisplay, ch.eglContext);
	eglDestroySurface(ch.eglDisplay, ch.eglSurface);
	eglTerminate(ch.eglDisplay);
}

bool make_current(const context_handle& ch)
{
	return (eglMakeCurrent(ch.eglDisplay, ch.eglSurface, ch.eglSurface, ch.eglContext) != 0);
}

void swap_buffers(const context_handle& ch)
{
	eglSwapBuffers(ch.eglDisplay, ch.eglSurface);
}

void recreate_surface(context_handle& ch, uint32_t /*width*/, uint32_t /*height*/)
{
	eglMakeCurrent(ch.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	if (ch.eglSurface != EGL_NO_SURFACE)
		eglDestroySurface(ch.eglDisplay, ch.eglSurface);

	ch.eglSurface = create_surface(ch.eglDisplay, ch.eglConfig);
}

}
}
}
