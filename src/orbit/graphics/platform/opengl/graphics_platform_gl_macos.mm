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

#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>

#include "orbit/core/platform/window_handle.h"

namespace orb
{
namespace platform
{
namespace gl
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

render_context_handle create_render_context_handle(const window_handle& wh)
{
	render_context_handle rch(in_place_type_v<render_context_handle::gl_t>);
	rch.gl.glView = create_open_gl_view((const NSWindow*)wh.nsWindow);
	return rch;
}

void destroy_context_handle(const window_handle& /*wh*/, const render_context_handle& rch)
{
	[(const NSOpenGLView*)rch.gl.glView removeFromSuperview];
	[(const NSOpenGLView*)rch.gl.glView dealloc];
}

bool make_current(const render_context_handle& rch)
{
	[[(const NSOpenGLView*)rch.gl.glView openGLContext] makeCurrentContext];
}

void swap_buffers(const render_context_handle& rch)
{
	[[(const NSOpenGLView*)rch.gl.glView openGLContext] flushBuffer];
}

void recreate_surface(render_context_handle& rch, uint32_t /*width*/, uint32_t /*height*/)
{
	[((NSOpenGLView*)rch.gl.glView).openGLContext update];
}

}
}
}
