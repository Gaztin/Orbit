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

#include "ConstantBuffer.h"

#include <cstring>

#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Core/Utility/Version.h"
#include "Orbit/Graphics/RenderContext.h"

ORB_NAMESPACE_BEGIN

template< typename T >
constexpr auto render_context_impl_index_v = unique_index_v< T, RenderContextImpl >;

template< typename T >
constexpr auto constant_buffer_impl_index_v = unique_index_v< T, ConstantBufferImpl >;

ConstantBuffer::ConstantBuffer( size_t size )
{
	auto context_impl_ptr = RenderContext::GetCurrent()->GetImplPtr();

	switch( context_impl_ptr->index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( render_context_impl_index_v< _RenderContextImplOpenGL > ):
		{
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( context_impl_ptr );
			if( (  context_impl->embedded && context_impl->opengl_version < Version( 3 ) ) ||
				( !context_impl->embedded && context_impl->opengl_version < Version( 3, 1 ) ) )
			{
				m_impl.emplace< _ConstantBufferImplOpenGL20 >();
			}
			else
			{
				m_impl.emplace< _ConstantBufferImplOpenGL31 >();
				auto  impl      = std::addressof( m_impl.emplace< _ConstantBufferImplOpenGL31 >() );
				auto& functions = context_impl->functions.value();

				functions.gen_buffers( 1, &impl->id );
				functions.bind_buffer( OpenGL::BufferTarget::Uniform, impl->id );
				functions.buffer_data( OpenGL::BufferTarget::Uniform, size, nullptr, OpenGL::BufferUsage::StreamDraw );
				functions.bind_buffer( OpenGL::BufferTarget::Uniform, 0 );
			}
			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( render_context_impl_index_v< _RenderContextImplD3D11 > ):
		{
			auto impl         = std::addressof( m_impl.emplace< _ConstantBufferImplD3D11 >() );
			auto context_impl = std::get_if< _RenderContextImplD3D11 >( context_impl_ptr );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth      = static_cast< UINT >( ( size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			ID3D11Buffer* buffer;
			context_impl->device->CreateBuffer( &desc, nullptr, &buffer );
			impl->buffer.reset( buffer );

			break;
		}
	#endif
	}
}

ConstantBuffer::~ConstantBuffer()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( constant_buffer_impl_index_v< _ConstantBufferImplOpenGL31 > ):
		{
			auto impl         = std::get_if< _ConstantBufferImplOpenGL31 >( &m_impl );
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetImplPtr() );

			context_impl->functions->delete_buffers( 1, &impl->id );

			break;
		}
	#endif
	}
}

void ConstantBuffer::Update( size_t location, const void* data, size_t size )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( constant_buffer_impl_index_v< _ConstantBufferImplOpenGL20 > ):
		{
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetImplPtr() );

			context_impl->functions->uniform1f( static_cast< GLint >( location ), *reinterpret_cast< const GLfloat* >( data ) );

			break;
		}
		case( constant_buffer_impl_index_v< _ConstantBufferImplOpenGL31 > ):
		{
			auto impl         = std::get_if< _ConstantBufferImplOpenGL31 >( &m_impl );
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetImplPtr() );

			context_impl->functions->bind_buffer( OpenGL::BufferTarget::Uniform, impl->id );

			void* dst = context_impl->functions->map_buffer_range( OpenGL::BufferTarget::Uniform, 0, size, OpenGL::MapAccess::WriteBit );
			std::memcpy( dst, data, size );

			context_impl->functions->unmap_buffer( OpenGL::BufferTarget::Uniform );
			context_impl->functions->bind_buffer( OpenGL::BufferTarget::Uniform, 0 );

			break;
		}
	#endif
		
	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( constant_buffer_impl_index_v< _ConstantBufferImplD3D11 > ):
		{
			auto impl         = std::get_if< _ConstantBufferImplD3D11 >( &m_impl );
			auto context_impl = std::get_if< _RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetImplPtr() );

			D3D11_MAPPED_SUBRESOURCE subresource;
			if( context_impl->device_context->Map( impl->buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource ) == S_OK )
			{
				std::memcpy( subresource.pData, data, size );
				context_impl->device_context->Unmap( impl->buffer.get(), 0 );
			}

			break;
		}
	#endif
	}
}

void ConstantBuffer::Bind( ShaderType type, uint32_t slot )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( constant_buffer_impl_index_v< _ConstantBufferImplOpenGL31 > ):
		{
			auto impl         = std::get_if< _ConstantBufferImplOpenGL31 >( &m_impl );
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetImplPtr() );

			context_impl->functions->bind_buffer( OpenGL::BufferTarget::Uniform, impl->id );
			context_impl->functions->bind_buffer_base( OpenGL::BufferTarget::Uniform, slot, impl->id );

			break;
		}
	#endif
		
	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( constant_buffer_impl_index_v< _ConstantBufferImplD3D11 > ):
		{
			auto          impl         = std::get_if< _ConstantBufferImplD3D11 >( &m_impl );
			auto          context_impl = std::get_if< _RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetImplPtr() );
			ID3D11Buffer* buffer       = impl->buffer.get();

			switch( type )
			{
				default: break;
				case ShaderType::Vertex:   context_impl->device_context->VSSetConstantBuffers( slot, 1, &buffer ); break;
				case ShaderType::Fragment: context_impl->device_context->PSSetConstantBuffers( slot, 1, &buffer ); break;
			}

			break;
		}
	#endif
	}
}

ORB_NAMESPACE_END
