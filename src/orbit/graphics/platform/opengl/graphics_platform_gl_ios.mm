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

#include <GLKit/GLKit.h>

#include "orbit/core/platform/window_handle.h"

@interface ORBGLKViewDelegate : UIResponder<GLKViewDelegate>
@end

namespace orb
{
namespace platform
{
namespace gl
{

render_context_handle create_render_context_handle(const window_handle& wh)
{
	render_context_handle rch(in_place_type_v<render_context_handle::gl_t>);
	rch.gl.eaglContext = [EAGLContext alloc];
	rch.gl.glkView = [GLKView alloc];

	ORBGLKViewDelegate* delegate = [ORBGLKViewDelegate alloc];
	[delegate init];

	[(EAGLContext*)rch.gl.eaglContext initWithAPI:kEAGLRenderingAPIOpenGLES1];
	[(GLKView*)rch.gl.glkView initWithFrame:[[UIScreen mainScreen] bounds]];
	((GLKView*)rch.gl.glkView).context = (EAGLContext*)rch.gl.eaglContext;
	((GLKView*)rch.gl.glkView).delegate = delegate;
	((GLKView*)rch.gl.glkView).enableSetNeedsDisplay = NO;
	[(UIWindow*)wh.uiWindow addSubview:(GLKView*)rch.gl.glkView];

	return rch;
}

void destroy_context_handle(const window_handle& /*wh*/, const render_context_handle& rch)
{
	[(GLKView*)rch.gl.glkView dealloc];
	[(EAGLContext*)rch.gl.eaglContext dealloc];
}

bool make_current(const render_context_handle& rch)
{
	return [EAGLContext setCurrentContext:(EAGLContext*)rch.gl.eaglContext];
}

void swap_buffers(const render_context_handle& rch)
{
	[(GLKView*)rch.gl.glkView display];
}

void recreate_surface(render_context_handle& rch, uint32_t width, uint32_t height)
{
	((GLKView*)rch.gl.glkView).layer.frame = CGRectMake(0.f, 0.f, width, height);
}

}
}
}

@implementation ORBGLKViewDelegate

- (void)glkView:(GLKView*)view drawInRect:(CGRect)rect
{
	(void)view;
	(void)rect;
}

@end
