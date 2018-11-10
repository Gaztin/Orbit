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

static GC create_gc(Display* display, const Window& window)
{
	return XCreateGC(display, window, 0, nullptr);
}

static GLXContext create_glx_context(Display* display)
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
	return glXCreateContext(display, visualInfo, nullptr, true);
}

context_handle create_context_handle(const window_handle& wh)
{
	context_handle ch{};
	ch.wndPtr = &wh;
	ch.gc = create_gc(wh.display, wh.window);
	ch.glxContext = create_glx_context(wh.display);
	return ch;
}

void destroy_context_handle(const window_handle& /*wh*/, const context_handle& ch)
{
	glXDestroyContext(ch.wndPtr->display, ch.glxContext);
	XFreeGC(ch.wndPtr->display, ch.gc);
}

bool make_current(const context_handle& ch)
{
	return glXMakeCurrent(ch.wndPtr->display, ch.wndPtr->window, ch.glxContext);
}

void swap_buffers(const context_handle& ch)
{
	glXSwapBuffers(ch.wndPtr->display, ch.wndPtr->window);
}

void recreate_surface(context_handle& /*ch*/)
{
}

}
}
}
