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

#include "render_context_d3d11.h"

#include <vector>

#include "orbit/core/platform/window_handle.h"
#include "orbit/core/memory.h"
#include "orbit/core/utility.h"

namespace orb
{
namespace platform
{

constexpr DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

static DXGI_RATIONAL find_monitor_refresh_rate(HWND hwnd)
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
		output->GetDisplayModeList(BackBufferFormat, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
		displayModes.resize(numDisplayModes);
		output->GetDisplayModeList(BackBufferFormat, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, &displayModes[0]);
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

static IDXGISwapChain* create_swap_chain(HWND hwnd)
{
	IDXGISwapChain* swapChain = nullptr;

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

	DXGI_SWAP_CHAIN_DESC desc{};
	desc.BufferDesc.Width = (windowRect.right - windowRect.left);
	desc.BufferDesc.Height = (windowRect.bottom - windowRect.top);
	desc.BufferDesc.RefreshRate = find_monitor_refresh_rate(hwnd);
	desc.BufferDesc.Format = BackBufferFormat;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 1;
	desc.OutputWindow = hwnd;
	desc.Windowed = true;

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

static ID3D11Device* create_device(IDXGISwapChain& swapChain)
{
	ID3D11Device* device;
	swapChain.GetDevice(__uuidof(ID3D11Device), cast<void**>(&device));
	return device;
}

static ID3D11DeviceContext* create_device_context(ID3D11Device& device)
{
	ID3D11DeviceContext* deviceContext;
	device.GetImmediateContext(&deviceContext);
	return deviceContext;
}

static ID3D11RenderTargetView* create_render_target_view(IDXGISwapChain& swapChain, ID3D11Device& device)
{
	ID3D11RenderTargetView* renderTargetView;
	ID3D11Texture2D* backBuffer;
	swapChain.GetBuffer(0, __uuidof(ID3D11Texture2D), cast<void**>(&backBuffer));
	device.CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
	backBuffer->Release();
	return renderTargetView;
}

static ID3D11Texture2D* create_depth_stencil_buffer(HWND hwnd, ID3D11Device& device)
{
	ID3D11Texture2D* depthStencilBuffer;
	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = (windowRect.right - windowRect.left);
	desc.Height = (windowRect.bottom - windowRect.top);
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DepthBufferFormat;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	device.CreateTexture2D(&desc, nullptr, &depthStencilBuffer);
	return depthStencilBuffer;
}

static ID3D11DepthStencilState* create_depth_stencil_state(ID3D11Device& device, ID3D11DeviceContext& deviceContext)
{
	ID3D11DepthStencilState* depthStencilState;
	D3D11_DEPTH_STENCIL_DESC desc{};
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.StencilEnable = true;
	desc.StencilReadMask = 0xff;
	desc.StencilWriteMask = 0xff;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	device.CreateDepthStencilState(&desc, &depthStencilState);
	deviceContext.OMSetDepthStencilState(depthStencilState, 1);
	return depthStencilState;
}

static ID3D11DepthStencilView* create_depth_stencil_view(ID3D11Device& device, ID3D11DeviceContext& deviceContext, ID3D11Texture2D& depthStencilBuffer, ID3D11RenderTargetView& renderTargetView)
{
	ID3D11DepthStencilView* depthStencilView;
	D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
	desc.Format = DepthBufferFormat;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	device.CreateDepthStencilView(&depthStencilBuffer, &desc, &depthStencilView);

	ID3D11RenderTargetView* renderTargets[] = { &renderTargetView };
	deviceContext.OMSetRenderTargets(1, renderTargets, depthStencilView);
	return depthStencilView;
}

static ID3D11RasterizerState* create_rasterization_state(ID3D11Device& device, ID3D11DeviceContext& deviceContext)
{
	ID3D11RasterizerState* rasterizationState;
	D3D11_RASTERIZER_DESC desc{};
	desc.CullMode = D3D11_CULL_BACK;
	desc.DepthClipEnable = true;
	desc.FillMode = D3D11_FILL_SOLID;
	device.CreateRasterizerState(&desc, &rasterizationState);
	deviceContext.RSSetState(rasterizationState);
	return rasterizationState;
}

render_context_d3d11::render_context_d3d11(const window_handle& wh)
	: m_parentHwnd(wh.hwnd)
	, m_swapChain(create_swap_chain(m_parentHwnd))
	, m_device(create_device(*m_swapChain))
	, m_deviceContext(create_device_context(*m_device))
	, m_renderTargetView(create_render_target_view(*m_swapChain, *m_device))
	, m_depthStencilBuffer(create_depth_stencil_buffer(m_parentHwnd, *m_device))
	, m_depthStencilState(create_depth_stencil_state(*m_device, *m_deviceContext))
	, m_depthStencilView(create_depth_stencil_view(*m_device, *m_deviceContext, *m_depthStencilBuffer, *m_renderTargetView))
	, m_rasterizerState(create_rasterization_state(*m_device, *m_deviceContext))
{
	m_deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void render_context_d3d11::resize(uint32_t width, uint32_t height)
{
	m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_deviceContext->ClearState();
	m_deviceContext->Flush();

	DXGI_SWAP_CHAIN_DESC desc;
	m_swapChain->GetDesc(&desc);
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	m_swapChain->ResizeBuffers(1, desc.BufferDesc.Width, desc.BufferDesc.Height, desc.BufferDesc.Format, desc.Flags);
	
	m_renderTargetView.reset(create_render_target_view(*m_swapChain, *m_device));
	m_depthStencilBuffer.reset(create_depth_stencil_buffer(m_parentHwnd, *m_device));
	m_depthStencilView.reset(create_depth_stencil_view(*m_device, *m_deviceContext, *m_depthStencilBuffer, *m_renderTargetView));

	ID3D11RenderTargetView* renderTargetViews = m_renderTargetView.get();
	m_deviceContext->OMSetRenderTargets(1, &renderTargetViews, m_depthStencilView.get());
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.get(), 0);
	m_deviceContext->RSSetState(m_rasterizerState.get());
}

void render_context_d3d11::swap_buffers()
{
	m_swapChain->Present(0, 0);
}

void render_context_d3d11::set_clear_color(float r, float g, float b)
{
	m_clearColor.r = r;
	m_clearColor.g = g;
	m_clearColor.b = b;
	m_clearColor.a = 1.0f;
}

void render_context_d3d11::clear_buffers(buffer_mask mask)
{
	if (!!(mask & buffer_mask::Color))
		m_deviceContext->ClearRenderTargetView(m_renderTargetView.get(), &m_clearColor[0]);
	if (!!(mask & buffer_mask::Depth))
		m_deviceContext->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

}
}
