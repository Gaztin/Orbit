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

#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>

#include "orbit/core/internal/window_impl.h"

namespace orb
{

render_context_opengl_impl::render_context_opengl_impl(const window_impl& parentWindowImpl)
{
	/* Choose pixel format. */
	const NSOpenGLPixelFormatAttribute attribs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFADepthSize, 24,
		0
	};
	NSOpenGLPixelFormat* pixelFormat = [NSOpenGLPixelFormat alloc];
	[pixelFormat initWithAttributes:attribs];
	
	/* Create OpenGL view. */
	NSView* view = [(NSWindow*)parentWindowImpl.window() contentView];
	NSOpenGLView* glView = [NSOpenGLView alloc];
	[glView initWithFrame:[view frame] pixelFormat:pixelFormat];
	[glView prepareOpenGL];
	[view addSubview:glView];
	m_glView = glView;
}

render_context_opengl_impl::~render_context_opengl_impl()
{
	[(NSOpenGLView*)m_glView removeFromSuperview];
	[(NSOpenGLView*)m_glView dealloc];
}

void render_context_opengl_impl::make_current(const window_impl&)
{
	[[(NSOpenGLView*)m_glView openGLContext] makeCurrentContext];
}

void render_context_opengl_impl::swap_buffers(const window_impl&)
{
	[[(NSOpenGLView*)m_glView openGLContext] flushBuffer];
}

void render_context_opengl_impl::reset_current()
{
	[NSOpenGLContext clearCurrentContext];
}

void render_context_opengl_impl::recreate_surface(const window_impl& parentWindowImpl)
{
}

bool render_context_opengl_impl::is_current() const
{
	return ([NSOpenGLContext currentContext] == [(NSOpenGLView*)m_glView openGLContext]);
}

}
