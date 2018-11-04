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
#include "orbit/core/memory.h"
#include "orbit/graphics/platform/context_base.h"

#include <d3d11.h>

namespace orb
{
namespace platform
{

class context_d3d11 : public context_base
{
public:
	explicit context_d3d11(const window_handle& wh);

	void resize(uint32_t width, uint32_t height) final override;
	void swap_buffers() final override;
	void clear(buffer_mask mask) final override;
	void set_clear_color(float r, float g, float b) final override;

private:
	window_handle m_parentWindowHandle;

	com_ptr<IDXGISwapChain> m_swapChain;
	com_ptr<ID3D11Device> m_device;
	com_ptr<ID3D11DeviceContext> m_deviceContext;
	com_ptr<ID3D11RenderTargetView> m_renderTargetView;
	com_ptr<ID3D11Texture2D> m_depthStencilBuffer;
	com_ptr<ID3D11DepthStencilState> m_depthStencilState;
	com_ptr<ID3D11DepthStencilView> m_depthStencilView;
	com_ptr<ID3D11RasterizerState> m_rasterizerState;

	float m_clearColor[4];
};

}
}
