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
#include "orbit/graphics/platform/d3d11/d3d11.h"
#include "orbit/graphics/platform/buffer_base.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
namespace platform
{
namespace d3d11
{

template<D3D11_BIND_FLAG BindFlag>
class buffer : public buffer_base
{
public:
	buffer(const void* data, size_t count, size_t size)
		: m_stride(static_cast<UINT>(size))
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = static_cast<UINT>(count * size);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = BindFlag;

		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = data;
		
		const render_context_handle& rch = render_context::get_current()->get_handle();
		ID3D11Buffer* buffer;
		rch.d3d11.device->CreateBuffer(&desc, &initialData, &buffer);
		m_buffer.reset(buffer);
	}

	void bind() final override;

private:
	com_ptr<ID3D11Buffer> m_buffer;
	UINT m_stride;
};

template<>
inline void buffer<D3D11_BIND_VERTEX_BUFFER>::bind()
{
	ID3D11DeviceContext* dc = render_context::get_current()->get_handle().d3d11.deviceContext;
	ID3D11Buffer* buffer = m_buffer.get();
	const UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, &buffer, &m_stride, &offset);
}

}
}
}
