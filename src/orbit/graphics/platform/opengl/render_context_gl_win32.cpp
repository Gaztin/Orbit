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

static int get_context_version_major(gl::version v)
{
	switch (v)
	{
		case gl::version::v2_0:
			return 2;

		case gl::version::v3_2:
			return 3;

		case gl::version::v4_1:
			return 4;

		default:
			return 0;
	}
}

static int get_context_version_minor(gl::version v)
{
	switch (v)
	{
		case gl::version::v2_0:
			return 0;

		case gl::version::v3_2:
			return 2;

		case gl::version::v4_1:
			return 1;

		default:
			return 0;
	}
}

static void set_pixel_format(HDC hdc)
{
	PIXELFORMATDESCRIPTOR desc{};
	desc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	desc.nVersion = 1;
	desc.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	desc.iPixelType = PFD_TYPE_RGBA;
	desc.cColorBits = 24;
	desc.cDepthBits = 32;
	desc.iLayerType = PFD_MAIN_PLANE;

	const int format = ChoosePixelFormat(hdc, &desc);
	SetPixelFormat(hdc, format, &desc);
}

render_context_gl::render_context_gl(const window_handle& wh, gl::version v)
	: m_parentHwnd(wh.hwnd)
	, m_hdc(GetDC(m_parentHwnd))
	, m_hglrc(nullptr)
	, m_dummyCtx(nullptr)
{
	set_pixel_format(m_hdc);

	m_dummyCtx = wglCreateContext(m_hdc);
	wglMakeCurrent(m_hdc, m_dummyCtx);

	using wglChoosePixelFormatARB_t    = BOOL (WINAPI*)(HDC hdc, const int* iAttribs, const FLOAT* fAttribs, UINT maxFormats, int* formats, UINT* numFormats);
	using wglCreateContextAttribsARB_t = HGLRC(WINAPI*)(HDC hdc, HGLRC shareContext, const int* attribs);

	wglChoosePixelFormatARB_t    wglChoosePixelFormatARB    = (wglChoosePixelFormatARB_t)wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB_t wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t)wglGetProcAddress("wglCreateContextAttribsARB");

	if (wglChoosePixelFormatARB && wglCreateContextAttribsARB)
	{
		const int formatAttributes[] =
		{
			0x2010, 1,      // WGL_SUPPORT_OPENGL_ARB
			0x2001, 1,      // WGL_DRAW_TO_WINDOW_ARB
			0x2002, 1,      // WGL_DRAW_TO_BITMAP_ARB
			0x2011, 1,      // WGL_DOUBLE_BUFFER_ARB
			0x2006, 1,      // WGL_SWAP_LAYER_BUFFERS_ARB
			0x2014, 24,     // WGL_COLOR_BITS_ARB
			0x2015, 8,      // WGL_RED_BITS_ARB
			0x2017, 8,      // WGL_GREEN_BITS_ARB
			0x2019, 8,      // WGL_BLUE_BITS_ARB
			0x201B, 0,      // WGL_ALPHA_BITS_ARB
			0x2022, 32,     // WGL_DEPTH_BITS_ARB
			0x2023, 8,      // WGL_STENCIL_BITS_ARB
			0x2003, 0x2027, // WGL_ACCELERATION_ARB = WGL_FULL_ACCELERATION_ARB
			0x2013, 0x202B, // WGL_PIXEL_TYPE_ARB = WGL_TYPE_RGBA_ARB
			0
		};

		int pixelFormats = 0;
		UINT pixelFormatCount = 0;
		wglChoosePixelFormatARB(m_hdc, formatAttributes, nullptr, 1, &pixelFormats, &pixelFormatCount);

		const int contextAttributes[] =
		{
			0x2091, get_context_version_major(v), // WGL_CONTEXT_MAJOR_VERSION_ARB
			0x2092, get_context_version_minor(v), // WGL_CONTEXT_MINOR_VERSION_ARB
			0x9126, 0x00000001,                   // WGL_CONTEXT_PROFILE_MASK_ARB = WGL_CONTEXT_CORE_PROFILE_BIT_ARB
			0
		};

		m_hglrc = wglCreateContextAttribsARB(m_hdc, nullptr, contextAttributes);
	}
	else
	{
		m_hglrc = wglCreateContext(m_hdc);
	}

	make_current();
	m_functions = gl::load_functions();
	make_current(nullptr);
}

render_context_gl::~render_context_gl()
{
	wglDeleteContext(m_hglrc);
	wglDeleteContext(m_dummyCtx);
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
