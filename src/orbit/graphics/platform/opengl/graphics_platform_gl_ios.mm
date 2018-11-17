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
#include "orbit/graphics/platform/opengl/context_handle_gl.h"

@interface ORBGLKViewDelegate : UIResponder<GLKViewDelegate>
@end

namespace orb
{
namespace platform
{
namespace gl
{

context_handle create_context_handle(const window_handle& wh)
{
	context_handle ch{};
	ch.eaglContext = [EAGLContext alloc];
	ch.glkView = [GLKView alloc];

	ORBGLKViewDelegate* delegate = [ORBGLKViewDelegate alloc];
	[delegate init];

	[(EAGLContext*)ch.eaglContext initWithAPI:kEAGLRenderingAPIOpenGLES1];
	[(GLKView*)ch.glkView initWithFrame:[[UIScreen mainScreen] bounds]];
	((GLKView*)ch.glkView).context = (EAGLContext*)ch.eaglContext;
	((GLKView*)ch.glkView).delegate = delegate;
	((GLKView*)ch.glkView).enableSetNeedsDisplay = NO;
	[(UIWindow*)wh.uiWindow addSubview:(GLKView*)ch.glkView];

	return ch;
}

void destroy_context_handle(const window_handle& /*wh*/, const context_handle& ch)
{
	[(GLKView*)ch.glkView dealloc];
	[(EAGLContext*)ch.eaglContext dealloc];
}

bool make_current(const context_handle& ch)
{
	return [EAGLContext setCurrentContext:(EAGLContext*)ch.eaglContext];
}

void swap_buffers(const context_handle& ch)
{
	[(GLKView*)ch.glkView display];
}

void recreate_surface(context_handle& ch, uint32_t width, uint32_t height)
{
	((GLKView*)ch.glkView).layer.frame = CGRectMake(0.f, 0.f, width, height);
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
