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

#include "constant_buffer_d3d11.h"

#include "orbit/graphics/platform/d3d11/render_context_d3d11.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
	namespace platform
	{
		using set_constant_buffers_t = void( STDMETHODCALLTYPE ID3D11DeviceContext::* )( UINT startSlot, UINT numBuffers, ID3D11Buffer* const* constantBuffers );

		static set_constant_buffers_t get_setter( shader_type type )
		{
			switch( type )
			{
				case shader_type::Vertex:   return &ID3D11DeviceContext::VSSetConstantBuffers;
				case shader_type::Fragment: return &ID3D11DeviceContext::PSSetConstantBuffers;
				default:                    return nullptr;
			}
		}

		constant_buffer_d3d11::constant_buffer_d3d11( size_t size )
			: m_buffer( d3d11::create_buffer( d3d11::bind_flag::ConstantBuffer, nullptr, size, d3d11::usage::Dynamic, d3d11::cpu_access::Write ) )
		{
		}

		void constant_buffer_d3d11::update( size_t /*location*/, const void* data, size_t size )
		{
			ID3D11DeviceContext& dc = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device_context();

			D3D11_MAPPED_SUBRESOURCE subresource = { };
			if( dc.Map( m_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource ) != S_OK )
				return;

			std::memcpy( subresource.pData, data, size );

			dc.Unmap( m_buffer.get(), 0 );
		}

		void constant_buffer_d3d11::bind( shader_type type, uint32_t slot )
		{
			ID3D11DeviceContext& dc        = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device_context();
			ID3D11Buffer*        buffers[] = { m_buffer.get() };

			std::invoke( get_setter( type ), &dc, slot, 1, buffers );
		}
	}
}
