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

#include "VertexBuffer.h"

#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/RenderContext.h"

ORB_NAMESPACE_BEGIN

template< typename T >
constexpr auto render_context_impl_index_v = unique_index_v< T, RenderContextImpl >;

template< typename T >
constexpr auto vertex_buffer_impl_index_v = unique_index_v< T, VertexBufferImpl >;

VertexBuffer::VertexBuffer( const void* data, size_t count, size_t stride )
	: m_impl  { }
	, m_count { count }
{
	auto         context_impl_ptr = RenderContext::GetCurrent()->GetImplPtr();
	const size_t total_size       = ( count * stride );

	switch( context_impl_ptr->index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( render_context_impl_index_v< _RenderContextImplOpenGL > ):
		{
			auto impl         = std::addressof( m_impl.emplace< _VertexBufferImplOpenGL >() );
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( context_impl_ptr );

			context_impl->functions->gen_buffers( 1, &impl->id );
			context_impl->functions->bind_buffer( OpenGL::BufferTarget::Array, impl->id );
			context_impl->functions->buffer_data( OpenGL::BufferTarget::Array, total_size, data, OpenGL::BufferUsage::StaticDraw );
			context_impl->functions->bind_buffer( OpenGL::BufferTarget::Array, 0 );

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( render_context_impl_index_v< _RenderContextImplD3D11 > ):
		{
			auto impl         = std::addressof( m_impl.emplace< _VertexBufferImplD3D11 >() );
			auto context_impl = std::get_if< _RenderContextImplD3D11 >( context_impl_ptr );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth = static_cast< UINT >( ( total_size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage     = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			ID3D11Buffer* buffer;
			if( data )
			{
				D3D11_SUBRESOURCE_DATA initial_data { };
				initial_data.pSysMem = data;
				context_impl->device->CreateBuffer( &desc, &initial_data, &buffer );
			}
			else
			{
				context_impl->device->CreateBuffer( &desc, nullptr, &buffer );
			}

			impl->buffer.reset( buffer );
			impl->stride = static_cast< UINT >( stride );

			break;
		}
	#endif
	}
}

VertexBuffer::~VertexBuffer()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( vertex_buffer_impl_index_v< _VertexBufferImplOpenGL > ):
		{
			auto impl         = std::get_if< _VertexBufferImplOpenGL >( &m_impl );
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetImplPtr() );

			context_impl->functions->delete_buffers( 1, &impl->id );

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( vertex_buffer_impl_index_v< _VertexBufferImplD3D11 > ):
		{
			break;
		}
	#endif
	}
}

void VertexBuffer::Bind()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( vertex_buffer_impl_index_v< _VertexBufferImplOpenGL > ):
		{
			auto impl         = std::get_if< _VertexBufferImplOpenGL >( &m_impl );
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetImplPtr() );

			context_impl->functions->bind_buffer( OpenGL::BufferTarget::Array, impl->id );

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( vertex_buffer_impl_index_v< _VertexBufferImplD3D11 > ):
		{
			auto          impl         = std::get_if< _VertexBufferImplD3D11 >( &m_impl );
			auto          context_impl = std::get_if< _RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetImplPtr() );
			ID3D11Buffer* buffer       = impl->buffer.get();
			UINT          stride       = impl->stride;
			UINT          offset       = 0;

			context_impl->device_context->IASetVertexBuffers( 0, 1, &buffer, &stride, &offset );

			break;
		}
	#endif
	}
}

ORB_NAMESPACE_END
