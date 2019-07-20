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

#include "constant_buffer.h"

#include "orbit/core/utility.h"
#include "orbit/core/version.h"
#include "orbit/graphics/render_context.h"

#include <cstring>

namespace orb
{
	template< typename T >
	constexpr auto render_context_impl_index_v = unique_index_v< T, render_context_impl >;
	
	template< typename T >
	constexpr auto constant_buffer_impl_index_v = unique_index_v< T, constant_buffer_impl >;

	constant_buffer::constant_buffer( size_t size )
	{
		auto currentContextImpl = render_context::get_current()->get_impl_ptr();

		switch( currentContextImpl->index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				auto implCtx = std::get_if< __render_context_impl_opengl >( currentContextImpl );
				if( ( implCtx->embedded && implCtx->glVersion < version( 3 ) ) || ( !implCtx->embedded && implCtx->glVersion < version( 3, 1 ) ) )
				{
					m_impl.emplace< __constant_buffer_impl_opengl_2_0 >();
				}
				else
				{
					auto  impl      = std::addressof( m_impl.emplace< __constant_buffer_impl_opengl_3_1 >() );
					auto& functions = implCtx->functions.value();

					functions.gen_buffers( 1, &impl->id );
					functions.bind_buffer( gl::buffer_target::Uniform, impl->id );
					functions.buffer_data( gl::buffer_target::Uniform, size, nullptr, orb::gl::buffer_usage::StreamDraw );
					functions.bind_buffer( gl::buffer_target::Uniform, 0 );
				}
				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
			{
				auto          impl   = std::addressof( m_impl.emplace< __constant_buffer_impl_d3d11 >() );
				ID3D11Device& device = *( std::get_if< __render_context_impl_d3d11 >( currentContextImpl )->device );

				D3D11_BUFFER_DESC desc{ };
				desc.ByteWidth      = static_cast< UINT >( ( size + 0xf ) & ~0xf ); /* Align by 16 bytes */
				desc.Usage          = D3D11_USAGE_DYNAMIC;
				desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				ID3D11Buffer* buffer;
				device.CreateBuffer( &desc, nullptr, &buffer );
				impl->buffer.reset( buffer );

				break;
			}
		#endif
		}
	}

	constant_buffer::~constant_buffer()
	{
		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( constant_buffer_impl_index_v< __constant_buffer_impl_opengl_3_1 > ):
			{
				auto  impl      = std::get_if< __constant_buffer_impl_opengl_3_1 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.delete_buffers( 1, &impl->id );

				break;
			}
		#endif
		}
	}

	void constant_buffer::update( void* dst, size_t location, const void* data, size_t size )
	{
		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( constant_buffer_impl_index_v< __constant_buffer_impl_opengl_2_0 > ):
			{
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.uniform1f( static_cast< GLint >( location ), *reinterpret_cast< const GLfloat* >( data ) );

				break;
			}
			case( constant_buffer_impl_index_v< __constant_buffer_impl_opengl_3_1 > ):
			{
				std::memcpy( dst, data, size );
				break;
			}
		#endif
			
		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( constant_buffer_impl_index_v< __constant_buffer_impl_d3d11 > ):
			{
				std::memcpy( dst, data, size );
				break;
			}
		#endif
		}
	}

	void* constant_buffer::update_begin( size_t size )
	{
		switch( m_impl.index() )
		{
			default: return nullptr;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( constant_buffer_impl_index_v< __constant_buffer_impl_opengl_3_1 > ):
			{
				auto  impl      = std::get_if< __constant_buffer_impl_opengl_3_1 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.bind_buffer( gl::buffer_target::Uniform, impl->id );
				return functions.map_buffer_range( gl::buffer_target::Uniform, 0, size, gl::map_access::WriteBit );
			}
		#endif
			
		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( constant_buffer_impl_index_v< __constant_buffer_impl_d3d11 > ):
			{
				auto                 impl          = std::get_if< __constant_buffer_impl_d3d11 >( &m_impl );
				ID3D11DeviceContext& deviceContext = *std::get_if< __render_context_impl_d3d11 >( render_context::get_current()->get_impl_ptr() )->deviceContext;

				D3D11_MAPPED_SUBRESOURCE subresource;
				if( deviceContext.Map( impl->buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource ) == S_OK )
					return subresource.pData;

				return nullptr;
			}
		#endif
		}
	}

	void constant_buffer::update_end()
	{
		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( constant_buffer_impl_index_v< __constant_buffer_impl_opengl_3_1 > ):
			{
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.unmap_buffer( gl::buffer_target::Uniform );
				functions.bind_buffer( gl::buffer_target::Uniform, 0 );

				break;
			}
		#endif
			
		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( constant_buffer_impl_index_v< __constant_buffer_impl_d3d11 > ):
			{
				auto                 impl          = std::get_if< __constant_buffer_impl_d3d11 >( &m_impl );
				ID3D11DeviceContext& deviceContext = *std::get_if< __render_context_impl_d3d11 >( render_context::get_current()->get_impl_ptr() )->deviceContext;

				deviceContext.Unmap( impl->buffer.get(), 0 );

				break;
			}
		#endif
		}
	}

	void constant_buffer::bind( [[maybe_unused]] shader_type type, uint32_t slot )
	{
		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( constant_buffer_impl_index_v< __constant_buffer_impl_opengl_3_1 > ):
			{
				auto  impl      = std::get_if< __constant_buffer_impl_opengl_3_1 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.bind_buffer( gl::buffer_target::Uniform, impl->id );
				functions.bind_buffer_base( gl::buffer_target::Uniform, slot, impl->id );

				break;
			}
		#endif
			
		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( constant_buffer_impl_index_v< __constant_buffer_impl_d3d11 > ):
			{
				auto                 impl          = std::get_if< __constant_buffer_impl_d3d11 >( &m_impl );
				ID3D11DeviceContext& deviceContext = *std::get_if< __render_context_impl_d3d11 >( render_context::get_current()->get_impl_ptr() )->deviceContext;
				ID3D11Buffer*        buffers[]     = { impl->buffer.get() };

				switch( type )
				{
					default: break;
					case shader_type::Vertex:   deviceContext.VSSetConstantBuffers( slot, 1, buffers ); break;
					case shader_type::Fragment: deviceContext.PSSetConstantBuffers( slot, 1, buffers ); break;
				}

				break;
			}
		#endif
		}
	}
}
