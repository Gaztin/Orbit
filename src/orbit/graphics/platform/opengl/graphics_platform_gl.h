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

#pragma once
#include "orbit/graphics/api/opengl/gl.h"
#include "orbit/graphics/platform/opengl/context_handle_gl.h"

namespace orb
{
namespace platform
{
struct window_handle;

namespace gl
{

inline GLbitfield get_buffer_bits(buffer_mask mask)
{
	GLbitfield bits = 0;
	bits |= (!!(mask & buffer_mask::Color)) ? GL_COLOR_BUFFER_BIT : 0;
	bits |= (!!(mask & buffer_mask::Depth)) ? GL_DEPTH_BUFFER_BIT : 0;
	return bits;
}

extern ORB_API_GRAPHICS context_handle create_context_handle(const window_handle& wh);
extern ORB_API_GRAPHICS void destroy_context_handle(const window_handle& wh, const context_handle& ch);
extern ORB_API_GRAPHICS bool make_current(const context_handle& ch);
extern ORB_API_GRAPHICS void swap_buffers(const context_handle& ch);
extern ORB_API_GRAPHICS void recreate_surface(context_handle& ch);

}
}
}
