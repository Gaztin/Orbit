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

#include <GLKit/GLKit.h>

#include "orbit/core/platform/window_handle.h"
#include "orbit/graphics/platform/opengl/gl_version.h"

@interface ORBGLKViewDelegate : UIResponder<GLKViewDelegate>
@end

namespace orb
{
namespace platform
{

static EAGLRenderingAPI get_eagl_rendering_api(gl::version v)
{
	switch (v)
	{
		case gl::version::vES_2:
			return kEAGLRenderingAPIOpenGLES2;

		case gl::version::vES_3:
			return kEAGLRenderingAPIOpenGLES3;

		default:
			return get_eagl_rendering_api(gl::get_system_default_opengl_version());
	}
}

render_context_gl::render_context_gl(const window_handle& wh, gl::version v)
	: m_eaglContext([EAGLContext alloc])
	, m_glkView([GLKView alloc])
{
	ORBGLKViewDelegate* delegate = [ORBGLKViewDelegate alloc];
	[delegate init];

	[(EAGLContext*)m_eaglContext initWithAPI:get_eagl_rendering_api(v)];
	[(GLKView*)m_glkView initWithFrame:[[UIScreen mainScreen] bounds]];
	((GLKView*)m_glkView).context = (EAGLContext*)m_eaglContext;
	((GLKView*)m_glkView).delegate = delegate;
	((GLKView*)m_glkView).enableSetNeedsDisplay = NO;
	[(UIWindow*)wh.uiWindow addSubview:(GLKView*)m_glkView];
	
	make_current();
	m_functions = gl::load_functions();
	make_current(nullptr);
}

render_context_gl::~render_context_gl()
{
	[(GLKView*)m_glkView dealloc];
	[(EAGLContext*)m_eaglContext dealloc];
}

bool render_context_gl::make_current()
{
	return [EAGLContext setCurrentContext:(EAGLContext*)m_eaglContext];
}

bool render_context_gl::make_current(std::nullptr_t)
{
	return [EAGLContext setCurrentContext:nullptr];
}

void render_context_gl::resize(uint32_t width, uint32_t height)
{
	((GLKView*)m_glkView).layer.frame = CGRectMake(0.f, 0.f, width, height);
	glViewport(0, 0, width, height);
}

void render_context_gl::swap_buffers()
{
	[(GLKView*)m_glkView display];
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

@implementation ORBGLKViewDelegate

- (void)glkView:(nonnull GLKView*)view drawInRect:(CGRect)rect
{
	/* Unused parameters */
	(void)view;
	(void)rect;
}

@end
