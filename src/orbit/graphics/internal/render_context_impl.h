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

#pragma once
#include "orbit.h"

#if defined(ORB_OS_WINDOWS)
#include <windows.h>
#include <gl/GL.h>
#elif defined(ORB_OS_ANDROID)
#include <EGL/egl.h>
#elif defined(ORB_OS_LINUX)
#include <GL/glx.h>
#endif

namespace orb
{

class window_impl;

class ORB_DLL_LOCAL render_context_impl
{
public:
	render_context_impl(const window_impl& parentWindowImpl);
	~render_context_impl();

	void make_current(const window_impl& parentWindowImpl);
	void swap_buffers(const window_impl& parentWindowImpl);

	bool is_current() const;

	static void reset_current();

private:
#if defined(ORB_OS_WINDOWS)
	HGLRC m_hglrc;

#elif defined(ORB_OS_ANDROID)
	EGLDisplay m_display;
	EGLSurface m_surface;
	EGLContext m_context;

#elif defined(ORB_OS_LINUX)
	Display*   m_display;
	Window     m_window;
	GC         m_gc;
	GLXContext m_context;

#elif defined(ORB_OS_MACOS)
	void* m_glView;
#endif
};

}
