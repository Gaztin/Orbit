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
#include "orbit/core/memory.h"
#include "orbit/graphics.h"

#if defined(ORB_OS_WINDOWS)
#include <d3d11.h>
#include <dxgi.h>
#endif

namespace orb
{
namespace d3d11
{

enum class bind_flag
{
	VertexBuffer    = 0x001,
	IndexBuffer     = 0x002,
	ConstantBuffer  = 0x004,
	ShaderResource  = 0x008,
	StreamOutput    = 0x010,
	RenderTarget    = 0x020,
	DepthStencil    = 0x040,
	UnorderedAccess = 0x080,
	Decoder         = 0x200,
	VideoEncoder    = 0x400,
};

enum class usage
{
	Default   = 0x0,
	Immutable = 0x1,
	Dynamic   = 0x2,
	Staging   = 0x3,
};

enum class cpu_access
{
	None  = 0x00000l,
	Write = 0x10000l,
	Read  = 0x20000l,
};

#if defined(ORB_OS_WINDOWS)
com_ptr<ID3D11Buffer> create_buffer(bind_flag bf, const void* data, size_t size, usage usg = usage::Default, cpu_access cpu = cpu_access::None);
#endif

}
}
