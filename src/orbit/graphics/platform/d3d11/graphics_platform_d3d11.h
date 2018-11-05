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
#include "orbit/graphics.h"

namespace orb
{
namespace platform
{
namespace d3d11
{

extern std::shared_ptr<IDXGISwapChain> create_swap_chain(const platform::window_handle& wh);
extern ID3D11Device& get_device(IDXGISwapChain& swapChain);
extern ID3D11DeviceContext& get_device_context(ID3D11Device& device);
extern std::shared_ptr<ID3D11RenderTargetView> create_render_target_view(IDXGISwapChain& swapChain, ID3D11Device& device);
extern std::shared_ptr<ID3D11Texture2D> create_depth_stencil_buffer(const platform::window_handle& wh, ID3D11Device& device);
extern std::shared_ptr<ID3D11DepthStencilState> create_depth_stencil_state(ID3D11Device& device, ID3D11DeviceContext& deviceContext);
extern std::shared_ptr<ID3D11DepthStencilView> create_depth_stencil_view(ID3D11Device& device, ID3D11DeviceContext& deviceContext, ID3D11Texture2D& depthStencilBuffer, ID3D11RenderTargetView& renderTargetView);
extern std::shared_ptr<ID3D11RasterizerState> create_rasterization_state(ID3D11Device& device, ID3D11DeviceContext& deviceContext);

}
}
}
