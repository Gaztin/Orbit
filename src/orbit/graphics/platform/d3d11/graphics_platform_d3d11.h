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
#include <memory>

#include <d3d11.h>
#include <dxgi.h>

#include "orbit/core/platform/window_handle.h"
#include "orbit/core/color.h"
#include "orbit/graphics/platform/d3d11/context_handle_d3d11.h"
#include "orbit/graphics/platform/d3d11/swap_chain_handle_d3d11.h"
#include "orbit/graphics.h"

namespace orb
{
namespace platform
{
namespace d3d11
{

extern swap_chain_handle create_swap_chain_handle(const window_handle& wh);
extern context_handle create_context_handle(const window_handle& wh, const swap_chain_handle& sch);
extern void flush_device_context(const swap_chain_handle& sch);
extern void resize_swap_chain(const swap_chain_handle& sch, uint32_t width, uint32_t height);
extern void recreate_buffers(context_handle& ch, const window_handle& wh, const swap_chain_handle& sch);
extern void present(const swap_chain_handle& sch);
extern void clear_render_target(const swap_chain_handle& sch, const context_handle& ch, const color& clr);
extern void clear_depth_stencil(const swap_chain_handle& sch, const context_handle& ch);

}
}
}
