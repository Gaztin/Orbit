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

#include "index_buffer.h"

#include "orbit/core/utility.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
	template< typename T >
	constexpr auto render_context_impl_index_v = unique_index_v< T, render_context_impl >;
	
	template< typename T >
	constexpr auto index_buffer_impl_index_v = unique_index_v< T, index_buffer_impl >;

	static size_t get_format_size( index_format fmt )
	{
		switch( fmt )
		{
			case orb::index_format::Byte:       return 1;
			case orb::index_format::Word:       return 2;
			case orb::index_format::DoubleWord: return 4;
			default:                            return 0;
		}
	}

	index_buffer::index_buffer( index_format fmt, const void* data, size_t count )
		: m_impl{ }
		, m_format( fmt )
		, m_count( count )
	{
		auto         currentContextImpl = render_context::get_current()->get_impl_ptr();
		const size_t totalSize          = ( count * get_format_size( fmt ) );

		switch( currentContextImpl->index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				auto  impl      = std::addressof( m_impl.emplace< __index_buffer_impl_opengl >() );
				auto& functions = std::get_if< __render_context_impl_opengl >( currentContextImpl )->functions;

				functions->gen_buffers( 1, &impl->id );
				functions->bind_buffer( gl::buffer_target::ElementArray, impl->id );
				functions->buffer_data( gl::buffer_target::ElementArray, totalSize, data, orb::gl::buffer_usage::StaticDraw );
				functions->bind_buffer( gl::buffer_target::ElementArray, 0 );

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
			{
				auto          impl   = std::addressof( m_impl.emplace< __index_buffer_impl_d3d11 >() );
				ID3D11Device& device = *( std::get_if< __render_context_impl_d3d11 >( currentContextImpl )->device );

				D3D11_BUFFER_DESC desc{ };
				desc.ByteWidth = static_cast< UINT >( ( totalSize + 0xf ) & ~0xf ); /* Align by 16 bytes */
				desc.Usage     = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

				ID3D11Buffer* buffer;
				if( data )
				{
					D3D11_SUBRESOURCE_DATA initialData{ };
					initialData.pSysMem = data;
					device.CreateBuffer( &desc, &initialData, &buffer );
				}
				else
				{
					device.CreateBuffer( &desc, nullptr, &buffer );
				}
				impl->buffer.reset( buffer );

				break;
			}
		#endif
		}
	}

	index_buffer::~index_buffer()
	{
		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( index_buffer_impl_index_v< __index_buffer_impl_opengl > ):
			{
				auto  impl      = std::get_if< __index_buffer_impl_opengl >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions;

				functions->delete_buffers( 1, &impl->id );

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
//			case( index_buffer_impl_index_v< __index_buffer_impl_d3d11 > ):
//			{
//				break;
//			}
		#endif
		}
	}

	void index_buffer::bind()
	{
		auto currentContextImpl = render_context::get_current()->get_impl_ptr();

		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( index_buffer_impl_index_v< __index_buffer_impl_opengl > ):
			{
				auto  impl      = std::get_if< __index_buffer_impl_opengl >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( currentContextImpl )->functions;

				functions->bind_buffer( gl::buffer_target::Array, impl->id );

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( index_buffer_impl_index_v< __index_buffer_impl_d3d11 > ):
			{
				auto                 impl          = std::get_if< __index_buffer_impl_d3d11 >( &m_impl );
				ID3D11DeviceContext& deviceContext = *( std::get_if< __render_context_impl_d3d11 >( currentContextImpl )->deviceContext );

				/* Translate format to DXGI_FORMAT */
				DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
				switch( m_format )
				{
					default:                                                      break;
					case index_format::Byte:       format = DXGI_FORMAT_R8_UINT;  break;
					case index_format::Word:       format = DXGI_FORMAT_R16_UINT; break;
					case index_format::DoubleWord: format = DXGI_FORMAT_R32_UINT; break;
				}

				deviceContext.IASetIndexBuffer( impl->buffer.get(), format, 0 );

				break;
			}
		#endif
		}
	}
}
