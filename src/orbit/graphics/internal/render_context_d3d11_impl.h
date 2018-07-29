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
#include <stdint.h>

#include "orbit/core/memory.h"

#if defined(ORB_OS_WINDOWS)
#include <d3d11.h>
#endif

namespace orb
{

class window_impl;

class ORB_DLL_LOCAL render_context_d3d11_impl
{
public:
	render_context_d3d11_impl(const window_impl& parentWindowImpl);

	void swap_buffers();
	void set_clear_color(float r, float g, float b, float a);
	void clear(uint32_t mask);

private:
#if defined(ORB_OS_WINDOWS)
	DXGI_RATIONAL find_monitor_refresh_rate(HWND hwnd) const;
	IDXGISwapChain* create_swap_chain(HWND hwnd) const;
	ID3D11Device* get_device() const;
	ID3D11DeviceContext* get_device_context() const;
	ID3D11RenderTargetView* create_render_target_view() const;
	ID3D11Texture2D* create_depth_stencil_buffer(HWND hwnd) const;
	ID3D11DepthStencilState* create_depth_stencil_state() const;
	ID3D11DepthStencilView* create_depth_stencil_view() const;
	ID3D11RasterizerState* create_rasterization_state() const;

	com_ptr<IDXGISwapChain> m_swapChain;
	com_ptr<ID3D11Device> m_device;
	com_ptr<ID3D11DeviceContext> m_deviceContext;
	com_ptr<ID3D11RenderTargetView> m_renderTargetView;
	com_ptr<ID3D11Texture2D> m_depthStencilBuffer;
	com_ptr<ID3D11DepthStencilState> m_depthStencilState;
	com_ptr<ID3D11DepthStencilView> m_depthStencilView;
	com_ptr<ID3D11RasterizerState> m_rasterizerState;
	float m_clearColor[4];
#endif
};

}
