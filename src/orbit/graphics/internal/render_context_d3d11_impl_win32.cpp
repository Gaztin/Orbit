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

#include "render_context_d3d11_impl.h"

#include "orbit/core/internal/window_impl.h"
#include "orbit/core/memory.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/render_context.h"

#include <vector>

namespace orb
{

constexpr DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

render_context_d3d11_impl::render_context_d3d11_impl(const window_impl& parentWindowImpl)
	: m_swapChain(create_swap_chain(parentWindowImpl.hwnd()))
	, m_device(get_device())
	, m_deviceContext(get_device_context())
	, m_renderTargetView(create_render_target_view())
	, m_depthStencilBuffer(create_depth_stencil_buffer(parentWindowImpl.hwnd()))
	, m_depthStencilState(create_depth_stencil_state())
	, m_depthStencilView(create_depth_stencil_view())
	, m_rasterizerState(create_rasterization_state())
	, m_clearColor{0.0f, 0.0f, 0.0f, 1.0f}
{
	RECT windowRect;
	GetWindowRect(parentWindowImpl.hwnd(), &windowRect);

	D3D11_VIEWPORT viewport { };
	viewport.Width    = cast<float>(windowRect.right - windowRect.left);
	viewport.Height   = cast<float>(windowRect.bottom - windowRect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_deviceContext->RSSetViewports(1, &viewport);
	m_deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void render_context_d3d11_impl::swap_buffers()
{
	m_swapChain->Present(0, 0);
}

void render_context_d3d11_impl::set_clear_color(float r, float g, float b, float a)
{
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
}

void render_context_d3d11_impl::clear(buffer_mask bm)
{
	if (!!(bm & buffer_mask::Color))
		m_deviceContext->ClearRenderTargetView(m_renderTargetView.get(), m_clearColor);
	if (!!(bm & buffer_mask::Depth))
		m_deviceContext->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void render_context_d3d11_impl::recreate_swap_chain(const window_impl& parentWindowImpl)
{
	m_deviceContext->OMSetRenderTargets(0, 0, 0);
	m_deviceContext->ClearState();
	m_deviceContext->Flush();

	RECT rect;
	GetWindowRect(parentWindowImpl.hwnd(), &rect);

	DXGI_SWAP_CHAIN_DESC desc;
	m_swapChain->GetDesc(&desc);
	desc.BufferDesc.Width  = (rect.right - rect.left);
	desc.BufferDesc.Height = (rect.bottom - rect.top);
	m_swapChain->ResizeBuffers(1, desc.BufferDesc.Width, desc.BufferDesc.Height, desc.BufferDesc.Format, desc.Flags);
	m_renderTargetView.reset(create_render_target_view());
	m_depthStencilBuffer.reset(create_depth_stencil_buffer(parentWindowImpl.hwnd()));
	m_depthStencilView.reset(create_depth_stencil_view());

	ID3D11RenderTargetView* renderTargetViews[1] = { m_renderTargetView.get() };
	m_deviceContext->OMSetRenderTargets(1, renderTargetViews, m_depthStencilView.get());
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.get(), 0);
	m_deviceContext->RSSetState(m_rasterizerState.get());
}

DXGI_RATIONAL render_context_d3d11_impl::find_monitor_refresh_rate(HWND hwnd) const
{
	com_ptr<IDXGIFactory> factory;
	com_ptr<IDXGIAdapter> adapter;
	com_ptr<IDXGIOutput> output;
	{ /* Get the primary adapter output. */
		IDXGIObject* tmp;
		CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&tmp));
		factory.reset(cast<IDXGIFactory*>(tmp));
		factory->EnumAdapters(0, cast<IDXGIAdapter**>(&tmp));
		adapter.reset(cast<IDXGIAdapter*>(tmp));
		adapter->EnumOutputs(0, cast<IDXGIOutput**>(&tmp));
		output.reset(cast<IDXGIOutput*>(tmp));
	}

	UINT monitorWidth, monitorHeight;
	{ /* Get monitor resolution. */
		HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfoA(monitor, &monitorInfo);
		monitorWidth = (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
		monitorHeight = (monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);
	}

	std::vector<DXGI_MODE_DESC> displayModes;
	{ /* Get display modes. */
		UINT numDisplayModes;
		output->GetDisplayModeList(backBufferFormat, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
		displayModes.resize(numDisplayModes);
		output->GetDisplayModeList(backBufferFormat, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, &displayModes[0]);
	}

	/* Finally, iterate all the display modes to find one matching the monitor's resolution. */
	for (DXGI_MODE_DESC& mode : displayModes)
	{
		if (mode.Width == monitorWidth &&
			mode.Height == monitorHeight)
		{
			return mode.RefreshRate;
		}
	}

	/* Default to 60Hz. */
	return { 60000, 1000 };
}

IDXGISwapChain* render_context_d3d11_impl::create_swap_chain(HWND hwnd) const
{
	IDXGISwapChain* swapChain;

	constexpr UINT deviceFlags = 0
#if !defined(NDEBUG)
		| D3D11_CREATE_DEVICE_DEBUG
#endif
		;
	const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	DXGI_SWAP_CHAIN_DESC desc { };
	desc.BufferDesc.Width       = (windowRect.right - windowRect.left);
	desc.BufferDesc.Height      = (windowRect.bottom - windowRect.top);
	desc.BufferDesc.RefreshRate = find_monitor_refresh_rate(hwnd);
	desc.BufferDesc.Format      = backBufferFormat;
	desc.SampleDesc.Count       = 1;
	desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount            = 1;
	desc.OutputWindow           = hwnd;
	desc.Windowed               = true;

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		deviceFlags,
		featureLevels,
		cast<UINT>(count_of(featureLevels)),
		D3D11_SDK_VERSION,
		&desc,
		&swapChain,
		nullptr,
		nullptr,
		nullptr
	);

	return swapChain;
}

ID3D11Device* render_context_d3d11_impl::get_device() const
{
	ID3D11Device* device;
	m_swapChain->GetDevice(__uuidof(ID3D11Device), cast<void**>(&device));
	return device;
}

ID3D11DeviceContext* render_context_d3d11_impl::get_device_context() const
{
	ID3D11DeviceContext* deviceContext;
	m_device->GetImmediateContext(&deviceContext);
	return deviceContext;
}

ID3D11RenderTargetView* render_context_d3d11_impl::create_render_target_view() const
{
	ID3D11RenderTargetView* renderTargetView;
	ID3D11Texture2D* backBuffer;
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), cast<void**>(&backBuffer));
	m_device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
	backBuffer->Release();
	return renderTargetView;
}

ID3D11Texture2D* render_context_d3d11_impl::create_depth_stencil_buffer(HWND hwnd) const
{
	ID3D11Texture2D* depthStencilBuffer;
	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	D3D11_TEXTURE2D_DESC desc { };
	desc.Width            = (windowRect.right - windowRect.left);
	desc.Height           = (windowRect.bottom - windowRect.top);
	desc.MipLevels        = 1;
	desc.ArraySize        = 1;
	desc.Format           = depthBufferFormat;
	desc.SampleDesc.Count = 1;
	desc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;
	m_device->CreateTexture2D(&desc, nullptr, &depthStencilBuffer);
	return depthStencilBuffer;
}

ID3D11DepthStencilState* render_context_d3d11_impl::create_depth_stencil_state() const
{
	ID3D11DepthStencilState* depthStencilState;
	D3D11_DEPTH_STENCIL_DESC desc { };
	desc.DepthEnable                  = true;
	desc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc                    = D3D11_COMPARISON_LESS;
	desc.StencilEnable                = true;
	desc.StencilReadMask              = 0xff;
	desc.StencilWriteMask             = 0xff;
	desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	desc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_DECR;
	desc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
	m_device->CreateDepthStencilState(&desc, &depthStencilState);
	m_deviceContext->OMSetDepthStencilState(depthStencilState, 1);
	return depthStencilState;
}

ID3D11DepthStencilView* render_context_d3d11_impl::create_depth_stencil_view() const
{
	ID3D11DepthStencilView* depthStencilView;
	D3D11_DEPTH_STENCIL_VIEW_DESC desc { };
	desc.Format        = depthBufferFormat;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	m_device->CreateDepthStencilView(m_depthStencilBuffer.get(), &desc, &depthStencilView);

	ID3D11RenderTargetView* renderTargets[] = { m_renderTargetView.get() };
	m_deviceContext->OMSetRenderTargets(1, renderTargets, depthStencilView);
	return depthStencilView;
}

ID3D11RasterizerState* render_context_d3d11_impl::create_rasterization_state() const
{
	ID3D11RasterizerState* rasterizationState;
	D3D11_RASTERIZER_DESC desc { };
	desc.CullMode        = D3D11_CULL_BACK;
	desc.DepthClipEnable = true;
	desc.FillMode        = D3D11_FILL_SOLID;
	m_device->CreateRasterizerState(&desc, &rasterizationState);
	m_deviceContext->RSSetState(rasterizationState);
	return rasterizationState;
}

}
