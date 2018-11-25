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

render_context_handle create_render_context_handle(const window_handle& wh)
{
	render_context_handle rch(in_place_type_v<render_context_handle::gl_t>);
	rch.gl.wndPtr = &wh;
	rch.gl.gc = create_gc(wh.display, wh.window);
	rch.gl.glxContext = create_glx_context(wh.display);
	return rch;
}

void destroy_context_handle(const window_handle& /*wh*/, const render_context_handle& rch)
{
	glXDestroyContext(rch.gl.wndPtr->display, rch.gl.glxContext);
	XFreeGC(rch.gl.wndPtr->display, rch.gl.gc);
}

bool make_current(const render_context_handle& rch)
{
	return glXMakeCurrent(rch.gl.wndPtr->display, rch.gl.wndPtr->window, rch.gl.glxContext);
}

void swap_buffers(const render_context_handle& rch)
{
	glXSwapBuffers(rch.gl.wndPtr->display, rch.gl.wndPtr->window);
}

void recreate_surface(render_context_handle& /*rch*/, uint32_t /*width*/, uint32_t /*height*/)
{
}

}
}
}
