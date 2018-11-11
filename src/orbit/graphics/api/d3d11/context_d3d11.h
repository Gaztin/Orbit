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
#include "orbit/core/platform/window_handle.h"
#include "orbit/core/color.h"
#include "orbit/core/memory.h"
#include "orbit/graphics/api/context_base.h"
#include "orbit/graphics/platform/d3d11/context_handle_d3d11.h"
#include "orbit/graphics/platform/d3d11/swap_chain_handle_d3d11.h"

namespace orb
{
namespace d3d11
{

class ORB_API_GRAPHICS context : public context_base
{
public:
	explicit context(const platform::window_handle& wh);

	void resize(uint32_t width, uint32_t height) final override;
	void swap_buffers() final override;
	void clear(buffer_mask mask) final override;
	void set_clear_color(float r, float g, float b) final override;

private:
	platform::window_handle m_parentWindowHandle;
	platform::d3d11::swap_chain_handle m_swapChainHandle;
	platform::d3d11::context_handle m_contextHandle;
	color m_clearColor;
};

}
}
