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

#include "graphics_platform_d3d11.h"

#include <vector>

#include <d3d11.h>

#include "orbit/core/platform/window_handle.h"
#include "orbit/core/memory.h"
#include "orbit/core/utility.h"

namespace orb
{
namespace platform
{
namespace d3d11
{

constexpr DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

static DXGI_RATIONAL find_monitor_refresh_rate(const platform::window_handle& wh)
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
		HMONITOR monitor = MonitorFromWindow(wh.hwnd, MONITOR_DEFAULTTONEAREST);
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

std::shared_ptr<IDXGISwapChain> create_swap_chain(const platform::window_handle& wh)
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
	GetWindowRect(wh.hwnd, &windowRect);

	DXGI_SWAP_CHAIN_DESC desc{};
	desc.BufferDesc.Width = (windowRect.right - windowRect.left);
	desc.BufferDesc.Height = (windowRect.bottom - windowRect.top);
	desc.BufferDesc.RefreshRate = find_monitor_refresh_rate(wh);
	desc.BufferDesc.Format = BackBufferFormat;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 1;
	desc.OutputWindow = wh.hwnd;
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

	return std::shared_ptr<IDXGISwapChain>(swapChain, com_deleter());
}

ID3D11Device& get_device(IDXGISwapChain& swapChain)
{
	ID3D11Device* device;
	swapChain.GetDevice(__uuidof(ID3D11Device), cast<void**>(&device));
	return *device;
}

ID3D11DeviceContext& get_device_context(ID3D11Device& device)
{
	ID3D11DeviceContext* deviceContext;
	device.GetImmediateContext(&deviceContext);
	return *deviceContext;
}

std::shared_ptr<ID3D11RenderTargetView> create_render_target_view(IDXGISwapChain& swapChain, ID3D11Device& device)
{
	ID3D11RenderTargetView* renderTargetView;
	ID3D11Texture2D* backBuffer;
	swapChain.GetBuffer(0, __uuidof(ID3D11Texture2D), cast<void**>(&backBuffer));
	device.CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
	backBuffer->Release();
	return std::shared_ptr<ID3D11RenderTargetView>(renderTargetView, com_deleter());
}

std::shared_ptr<ID3D11Texture2D> create_depth_stencil_buffer(const platform::window_handle& wh, ID3D11Device& device)
{
	ID3D11Texture2D* depthStencilBuffer;
	RECT windowRect;
	GetWindowRect(wh.hwnd, &windowRect);

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = (windowRect.right - windowRect.left);
	desc.Height = (windowRect.bottom - windowRect.top);
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DepthBufferFormat;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	device.CreateTexture2D(&desc, nullptr, &depthStencilBuffer);
	return std::shared_ptr<ID3D11Texture2D>(depthStencilBuffer, com_deleter());
}

std::shared_ptr<ID3D11DepthStencilState> create_depth_stencil_state(ID3D11Device& device, ID3D11DeviceContext& deviceContext)
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
	return std::shared_ptr<ID3D11DepthStencilState>(depthStencilState, com_deleter());
}

std::shared_ptr<ID3D11DepthStencilView> create_depth_stencil_view(ID3D11Device& device, ID3D11DeviceContext& deviceContext, ID3D11Texture2D& depthStencilBuffer, ID3D11RenderTargetView& renderTargetView)
{
	ID3D11DepthStencilView* depthStencilView;
	D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
	desc.Format = DepthBufferFormat;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	device.CreateDepthStencilView(&depthStencilBuffer, &desc, &depthStencilView);

	ID3D11RenderTargetView* renderTargets[] = { &renderTargetView };
	deviceContext.OMSetRenderTargets(1, renderTargets, depthStencilView);
	return std::shared_ptr<ID3D11DepthStencilView>(depthStencilView, com_deleter());
}

std::shared_ptr<ID3D11RasterizerState> create_rasterization_state(ID3D11Device& device, ID3D11DeviceContext& deviceContext)
{
	ID3D11RasterizerState* rasterizationState;
	D3D11_RASTERIZER_DESC desc{};
	desc.CullMode = D3D11_CULL_BACK;
	desc.DepthClipEnable = true;
	desc.FillMode = D3D11_FILL_SOLID;
	device.CreateRasterizerState(&desc, &rasterizationState);
	deviceContext.RSSetState(rasterizationState);
	return std::shared_ptr<ID3D11RasterizerState>(rasterizationState, com_deleter());
}

}
}
}
