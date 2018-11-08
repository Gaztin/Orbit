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

#include "context_d3d11.h"

#include <vector>

#include "orbit/core/utility.h"
#include "orbit/graphics/platform/d3d11/graphics_platform_d3d11.h"

namespace orb
{
namespace d3d11
{

context::context(const platform::window_handle& wh)
	: m_parentWindowHandle(wh)
	, m_swapChainHandle(platform::d3d11::create_swap_chain_handle(wh))
	, m_contextHandle(platform::d3d11::create_context_handle(wh, m_swapChainHandle))
{
}

void context::resize(uint32_t width, uint32_t height)
{
	platform::d3d11::flush_device_context(m_swapChainHandle);
	platform::d3d11::resize_swap_chain(m_swapChainHandle, width, height);
	platform::d3d11::recreate_buffers(m_contextHandle, m_parentWindowHandle, m_swapChainHandle);
}

void context::swap_buffers()
{
	platform::d3d11::present(m_swapChainHandle);
}

void context::clear(buffer_mask mask)
{
	if (!!(mask & buffer_mask::Color))
		platform::d3d11::clear_render_target(m_swapChainHandle, m_contextHandle, m_clearColor);
	if (!!(mask & buffer_mask::Depth))
		platform::d3d11::clear_depth_stencil(m_swapChainHandle, m_contextHandle);
}

void context::set_clear_color(float r, float g, float b)
{
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
}

}
}
