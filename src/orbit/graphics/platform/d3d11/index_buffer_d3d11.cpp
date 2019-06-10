/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "index_buffer_d3d11.h"

namespace orb
{
	namespace platform
	{
		static size_t format_size( index_format fmt )
		{
			switch( fmt )
			{
				case index_format::Byte:       return 1;
				case index_format::Word:       return 2;
				case index_format::DoubleWord: return 4;
				default:                       return 0;
			}
		}

		static DXGI_FORMAT dxgi_format( index_format fmt )
		{
			switch( fmt )
			{
				case index_format::Byte:       return DXGI_FORMAT_R8_UINT;
				case index_format::Word:       return DXGI_FORMAT_R16_UINT;
				case index_format::DoubleWord: return DXGI_FORMAT_R32_UINT;
				default:                       return DXGI_FORMAT_UNKNOWN;
			}
		}

		index_buffer_d3d11::index_buffer_d3d11( index_format fmt, const void* data, size_t count )
			: m_buffer( d3d11::create_buffer( d3d11::bind_flag::IndexBuffer, data, count* format_size( fmt ) ) )
			, m_format( dxgi_format( fmt ) )
		{
		}

		void index_buffer_d3d11::bind()
		{
			ID3D11DeviceContext& dc = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device_context();
			dc.IASetIndexBuffer( m_buffer.get(), m_format, 0 );
		}
	}
}
