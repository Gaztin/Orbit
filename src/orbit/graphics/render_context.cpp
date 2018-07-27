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

#include "render_context.h"

#include <assert.h>

#include "orbit/graphics/internal/render_context_impl.h"
#include "orbit/core/internal/window_impl.h"
#include "orbit/core/window.h"

namespace orb
{

static GLbitfield buffer_mask_value(uint32_t mask)
{
	GLbitfield bitfield = 0;
	bitfield |= (mask & buffer_mask::Color   & ~0) ? GL_COLOR_BUFFER_BIT   : 0;
	bitfield |= (mask & buffer_mask::Depth   & ~0) ? GL_DEPTH_BUFFER_BIT   : 0;
	bitfield |= (mask & buffer_mask::Accum   & ~0) ? GL_ACCUM_BUFFER_BIT   : 0;
	bitfield |= (mask & buffer_mask::Stencil & ~0) ? GL_STENCIL_BUFFER_BIT : 0;
	return bitfield;
}

render_context::render_context(const window& parentWindow)
	: opaque_memory(parentWindow.impl())
{
}

render_context::~render_context()
{
	if (is_current())
		reset_current();
}

void render_context::make_current(const window& parentWindow)
{
	impl().make_current(parentWindow.impl());
}

void render_context::swap_buffers(const window& parentWindow)
{
	impl().swap_buffers(parentWindow.impl());
}

void render_context::clear(uint32_t mask)
{
	assert(is_current());
	glClear(buffer_mask_value(mask));
}

void render_context::set_clear_color(float r, float g, float b)
{
	assert(is_current());
	glClearColor(r, g, b, 1.0f);
}

bool render_context::is_current() const
{
	return impl().is_current();
}

void render_context::reset_current()
{
	render_context_impl::reset_current();
}

}
