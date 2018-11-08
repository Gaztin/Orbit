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

#include <dxgi.h>
#include <d3d11.h>

#include "orbit/graphics.h"

namespace orb
{
namespace platform
{
namespace d3d11
{

struct swap_chain_handle
{
#if defined(ORB_OS_WINDOWS)
	std::shared_ptr<IDXGISwapChain> swapChain;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
#endif
};

}
}
}
