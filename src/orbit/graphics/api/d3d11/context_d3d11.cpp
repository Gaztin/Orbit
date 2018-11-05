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
	, m_swapChain(platform::d3d11::create_swap_chain(wh))
	, m_device(platform::d3d11::get_device(*m_swapChain))
	, m_deviceContext(platform::d3d11::get_device_context(m_device))
	, m_renderTargetView(platform::d3d11::create_render_target_view(*m_swapChain, m_device))
	, m_depthStencilBuffer(platform::d3d11::create_depth_stencil_buffer(wh, m_device))
	, m_depthStencilState(platform::d3d11::create_depth_stencil_state(m_device, m_deviceContext))
	, m_depthStencilView(platform::d3d11::create_depth_stencil_view(m_device, m_deviceContext, *m_depthStencilBuffer, *m_renderTargetView))
	, m_rasterizerState(platform::d3d11::create_rasterization_state(m_device, m_deviceContext))
	, m_clearColor{ 0.0f, 0.0f, 0.0f, 1.0f }
{
	RECT windowRect;
	GetWindowRect(wh.hwnd, &windowRect);

	D3D11_VIEWPORT viewport{};
	viewport.Width = cast<float>(windowRect.right - windowRect.left);
	viewport.Height = cast<float>(windowRect.bottom - windowRect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_deviceContext.RSSetViewports(1, &viewport);
	m_deviceContext.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void context::resize(uint32_t width, uint32_t height)
{
	m_deviceContext.OMSetRenderTargets(0, 0, 0);
	m_deviceContext.ClearState();
	m_deviceContext.Flush();

	DXGI_SWAP_CHAIN_DESC desc;
	m_swapChain->GetDesc(&desc);
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	m_swapChain->ResizeBuffers(1, desc.BufferDesc.Width, desc.BufferDesc.Height, desc.BufferDesc.Format, desc.Flags);
	m_renderTargetView = platform::d3d11::create_render_target_view(*m_swapChain, m_device);
	m_depthStencilBuffer = platform::d3d11::create_depth_stencil_buffer(m_parentWindowHandle, m_device);
	m_depthStencilView = platform::d3d11::create_depth_stencil_view(m_device, m_deviceContext, *m_depthStencilBuffer, *m_renderTargetView);

	ID3D11RenderTargetView* renderTargetViews[1] = { m_renderTargetView.get() };
	m_deviceContext.OMSetRenderTargets(1, renderTargetViews, m_depthStencilView.get());
	m_deviceContext.OMSetDepthStencilState(m_depthStencilState.get(), 0);
	m_deviceContext.RSSetState(m_rasterizerState.get());
}

void context::swap_buffers()
{
	m_swapChain->Present(0, 0);
}

void context::clear(buffer_mask mask)
{
	if (!!(mask & buffer_mask::Color))
		m_deviceContext.ClearRenderTargetView(m_renderTargetView.get(), m_clearColor);
	if (!!(mask & buffer_mask::Depth))
		m_deviceContext.ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void context::set_clear_color(float r, float g, float b)
{
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
}

}
}
