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

#include "orbit/core/platform/window_handle.h"

namespace orb
{
namespace platform
{
namespace gl
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

render_context_handle create_render_context_handle(const window_handle& wh)
{
	render_context_handle rch(in_place_type_v<render_context_handle::gl_t>);
	rch.gl.hdc = GetDC(wh.hwnd);
	set_pixel_format(rch.gl.hdc);
	rch.gl.hglrc = wglCreateContext(rch.gl.hdc);
	return rch;
}

void destroy_context_handle(const window_handle& wh, const render_context_handle& rch)
{
	wglDeleteContext(rch.gl.hglrc);
	ReleaseDC(wh.hwnd, rch.gl.hdc);
}

bool make_current(const render_context_handle& rch)
{
	return wglMakeCurrent(rch.gl.hdc, rch.gl.hglrc);
}

void swap_buffers(const render_context_handle& rch)
{
	SwapBuffers(rch.gl.hdc);
}

void recreate_surface(render_context_handle& /*ch*/, uint32_t /*width*/, uint32_t /*height*/)
{
}

}
}
}
