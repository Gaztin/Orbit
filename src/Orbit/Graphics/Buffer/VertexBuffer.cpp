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
#include "Orbit/Graphics/Device/RenderContext.h"

ORB_NAMESPACE_BEGIN

VertexBuffer::VertexBuffer( const void* data, size_t count, size_t stride )
	: m_impl  { }
	, m_count { count }
{
	auto&        context_impl_var = RenderContext::GetInstance().GetPrivateImpl();
	const size_t total_size       = ( count * stride );

	switch( context_impl_var.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextImplOpenGL, Private::RenderContextImpl > ):
		{
			auto& impl         = m_impl.emplace< Private::_VertexBufferImplOpenGL >();
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( context_impl_var );

			context_impl.functions->gen_buffers( 1, &impl.id );
			context_impl.functions->bind_buffer( OpenGL::BufferTarget::Array, impl.id );
			context_impl.functions->buffer_data( OpenGL::BufferTarget::Array, total_size, data, OpenGL::BufferUsage::StaticDraw );
			context_impl.functions->bind_buffer( OpenGL::BufferTarget::Array, 0 );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextImplD3D11, Private::RenderContextImpl > ):
		{
			auto& impl         = m_impl.emplace< Private::_VertexBufferImplD3D11 >();
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( context_impl_var );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth = static_cast< UINT >( ( total_size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage     = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			ID3D11Buffer* buffer;
			if( data )
			{
				D3D11_SUBRESOURCE_DATA initial_data { };
				initial_data.pSysMem = data;
				context_impl.device->CreateBuffer( &desc, &initial_data, &buffer );
			}
			else
			{
				context_impl.device->CreateBuffer( &desc, nullptr, &buffer );
			}

			impl.buffer.reset( buffer );
			impl.stride = static_cast< UINT >( stride );

			break;
		}

	#endif

	}
}

VertexBuffer::~VertexBuffer( void )
{
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferImplOpenGL, Private::VertexBufferImpl > ):
		{
			auto& impl         = std::get< Private::_VertexBufferImplOpenGL >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetInstance().GetPrivateImpl() );

			context_impl.functions->delete_buffers( 1, &impl.id );

			break;
		}

	#endif

	}
}

void VertexBuffer::Bind( void )
{
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferImplOpenGL, Private::VertexBufferImpl > ):
		{
			auto& impl         = std::get< Private::_VertexBufferImplOpenGL >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetInstance().GetPrivateImpl() );

			context_impl.functions->bind_buffer( OpenGL::BufferTarget::Array, impl.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_VertexBufferImplD3D11, Private::VertexBufferImpl > ):
		{
			auto&         impl         = std::get< Private::_VertexBufferImplD3D11 >( m_impl );
			auto&         context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetInstance().GetPrivateImpl() );
			ID3D11Buffer* buffer       = impl.buffer.get();
			UINT          stride       = impl.stride;
			UINT          offset       = 0;

			context_impl.device_context->IASetVertexBuffers( 0, 1, &buffer, &stride, &offset );

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
