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
#include <cstddef>

#include "orbit/core/memory.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/platform/d3d11/d3d11.h"
#include "orbit/graphics/platform/d3d11/render_context_d3d11.h"
#include "orbit/graphics/platform/buffer_base.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
namespace platform
{

template<d3d11::bind_flag BindFlag>
class buffer_d3d11 : public buffer_base
{
public:
	buffer_d3d11(const void* data, size_t count, size_t size)
#if defined(ORB_OS_WINDOWS)
		: m_stride(static_cast<UINT>(size))
#endif
	{
#if defined(ORB_OS_WINDOWS)
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = static_cast<UINT>(count * size);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = static_cast<UINT>(BindFlag);

		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = data;
		
		ID3D11Device& device = cast<render_context_d3d11&, render_context_base&>(render_context::get_current()->get_base()).get_device();
		ID3D11Buffer* buffer;
		device.CreateBuffer(&desc, &initialData, &buffer);
		m_buffer.reset(buffer);
#else
		/* Unused parameters */
		(void)data;
		(void)count;
		(void)size;
#endif
	}

	void bind() final override;

private:
#if defined(ORB_OS_WINDOWS)
	com_ptr<ID3D11Buffer> m_buffer;
	UINT m_stride;
#endif
};

template<>
inline void buffer_d3d11<d3d11::bind_flag::VertexBuffer>::bind()
{
#if defined(ORB_OS_WINDOWS)
	ID3D11DeviceContext& dc = cast<render_context_d3d11&, render_context_base&>(render_context::get_current()->get_base()).get_device_context();
	ID3D11Buffer* buffer = m_buffer.get();
	const UINT offset = 0;
	dc.IASetVertexBuffers(0, 1, &buffer, &m_stride, &offset);
#endif
}

}
}
