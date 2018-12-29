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

#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>

#include "orbit/core/platform/window_handle.h"

namespace orb
{
namespace platform
{

static NSOpenGLView* create_open_gl_view(const NSWindow* nsWindow)
{
	const NSOpenGLPixelFormatAttribute Attribs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFADepthSize, 24,
		0
	};

	NSOpenGLPixelFormat* pixelFormat = [NSOpenGLPixelFormat alloc];
	[pixelFormat initWithAttributes:Attribs];

	NSOpenGLView* glView = [NSOpenGLView alloc];
	[glView initWithFrame:nsWindow.contentView.frame pixelFormat:pixelFormat];
	[glView prepareOpenGL];
	[nsWindow.contentView addSubview:glView];

	return glView;
}

render_context_gl::render_context_gl(const window_handle& wh)
	: m_glView(create_open_gl_view((const NSWindow*)wh.nsWindow))
{
	make_current();
	m_functions = gl::load_functions();
	make_current(nullptr);
}

render_context_gl::~render_context_gl()
{
	[(const NSOpenGLView*)m_glView removeFromSuperview];
	[(const NSOpenGLView*)m_glView dealloc];
}

bool render_context_gl::make_current()
{
	[[(const NSOpenGLView*)m_glView openGLContext] makeCurrentContext];
	return true;
}

bool render_context_gl::make_current(std::nullptr_t)
{
	[NSOpenGLContext clearCurrentContext];
	return true;
}

void render_context_gl::resize(uint32_t width, uint32_t height)
{
	[((NSOpenGLView*)m_glView).openGLContext update];
	glViewport(0, 0, width, height);
}

void render_context_gl::swap_buffers()
{
	[[(const NSOpenGLView*)m_glView openGLContext] flushBuffer];
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
