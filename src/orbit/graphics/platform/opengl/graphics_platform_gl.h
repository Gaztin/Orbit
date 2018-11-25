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
#include "orbit/graphics/platform/render_context_handle.h"
#include "orbit/graphics/gl.h"

namespace orb
{
namespace platform
{
struct window_handle;

namespace gl
{

extern ORB_API_GRAPHICS render_context_handle create_render_context_handle(const window_handle& wh);
extern ORB_API_GRAPHICS void destroy_context_handle(const window_handle& wh, const render_context_handle& rch);
extern ORB_API_GRAPHICS bool make_current(const render_context_handle& rch);
extern ORB_API_GRAPHICS void swap_buffers(const render_context_handle& rch);
extern ORB_API_GRAPHICS void recreate_surface(render_context_handle& rch, uint32_t width, uint32_t height);

}
}
}
