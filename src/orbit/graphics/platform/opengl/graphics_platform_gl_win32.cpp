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

context_handle create_context_handle(const window_handle& wh)
{
	context_handle ch{};
	ch.hdc = GetDC(wh.hwnd);
	ch.hglrc = wglCreateContext(ch.hdc);
	return ch;
}

void destroy_context_handle(const window_handle& wh, const context_handle& ch)
{
	wglDeleteContext(ch.hglrc);
	ReleaseDC(wh.hwnd, ch.hdc);
}

bool make_current(const context_handle& ch)
{
	return wglMakeCurrent(ch.hdc, ch.hglrc);
}

void swap_buffers(const context_handle& ch)
{
	SwapBuffers(ch.hdc);
}

void recreate_surface(context_handle& /*ch*/, uint32_t /*width*/, uint32_t /*height*/)
{
}

}
}
}
