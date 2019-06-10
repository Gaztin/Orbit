/*
* Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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

#include "vertex_buffer_d3d11.h"

#include "orbit/graphics/platform/d3d11/d3d11.h"
#include "orbit/graphics/platform/d3d11/render_context_d3d11.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
	namespace platform
	{
		vertex_buffer_d3d11::vertex_buffer_d3d11( const void* data, size_t count, size_t stride )
			: m_buffer( d3d11::create_buffer( d3d11::bind_flag::VertexBuffer, data, count* stride ) )
			, m_stride( static_cast< UINT >( stride ) )
		{
		}

		void vertex_buffer_d3d11::bind()
		{
			ID3D11DeviceContext& dc        = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device_context();
			ID3D11Buffer*        buffers[] = { m_buffer.get() };
			const UINT           strides[] = { m_stride };
			const UINT           offsets[] = { 0 };
			dc.IASetVertexBuffers( 0, 1, buffers, strides, offsets );
		}
	}
}
