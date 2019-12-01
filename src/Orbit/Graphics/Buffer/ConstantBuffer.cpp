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
#include "Orbit/Graphics/Device/RenderContext.h"

ORB_NAMESPACE_BEGIN

ConstantBuffer::ConstantBuffer( size_t size )
{
	auto& context_impl_var = RenderContext::GetCurrent()->GetPrivateImpl();

	switch( context_impl_var.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL

		case( unique_index_v< Private::_RenderContextImplOpenGL, Private::RenderContextImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( context_impl_var );

			if( (  context_impl.embedded && context_impl.opengl_version < Version( 3 ) )||
			    ( !context_impl.embedded && context_impl.opengl_version < Version( 3, 1 ) ) )
			{
				m_impl.emplace< Private::_ConstantBufferImplOpenGL20 >();
			}
			else
			{
				auto& impl = m_impl.emplace< Private::_ConstantBufferImplOpenGL31 >();

				context_impl.functions->gen_buffers( 1, &impl.id );
				context_impl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, impl.id );
				context_impl.functions->buffer_data( OpenGL::BufferTarget::Uniform, size, nullptr, OpenGL::BufferUsage::StreamDraw );
				context_impl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, 0 );
			}
			break;
		}

	#endif
	#if _ORB_HAS_GRAPHICS_API_D3D11

		case( unique_index_v< Private::_RenderContextImplD3D11, Private::RenderContextImpl > ):
		{
			auto& impl         = m_impl.emplace< Private::_ConstantBufferImplD3D11 >();
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( context_impl_var );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth      = static_cast< UINT >( ( size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			ID3D11Buffer* buffer;
			context_impl.device->CreateBuffer( &desc, nullptr, &buffer );
			impl.buffer.reset( buffer );

			break;
		}

	#endif

	}
}

ConstantBuffer::~ConstantBuffer( void )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL

		case( unique_index_v< Private::_ConstantBufferImplOpenGL31, Private::ConstantBufferImpl > ):
		{
			auto& impl         = std::get< Private::_ConstantBufferImplOpenGL31 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->delete_buffers( 1, &impl.id );

			break;
		}

	#endif

	}
}

void ConstantBuffer::Update( void* dst, size_t location, const void* data, size_t size )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL

		case( unique_index_v< Private::_ConstantBufferImplOpenGL20, Private::ConstantBufferImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->uniform1f( static_cast< GLint >( location ), *reinterpret_cast< const GLfloat* >( data ) );

			break;
		}

		case( unique_index_v< Private::_ConstantBufferImplOpenGL31, Private::ConstantBufferImpl > ):
		{
			std::memcpy( dst, data, size );

			break;
		}

	#endif
	#if _ORB_HAS_GRAPHICS_API_D3D11

		case( unique_index_v< Private::_ConstantBufferImplD3D11, Private::ConstantBufferImpl > ):
		{
			std::memcpy( dst, data, size );

			break;
		}

	#endif

	}
}

void* ConstantBuffer::UpdateBegin( size_t size )
{
	switch( m_impl.index() )
	{
		default: return nullptr;

	#if _ORB_HAS_GRAPHICS_API_OPENGL

		case( unique_index_v< Private::_ConstantBufferImplOpenGL31, Private::ConstantBufferImpl > ):
		{
			auto& impl         = std::get< Private::_ConstantBufferImplOpenGL31 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, impl.id );
			return context_impl.functions->map_buffer_range( OpenGL::BufferTarget::Uniform, 0, size, OpenGL::MapAccess::WriteBit );
		}

	#endif
	#if _ORB_HAS_GRAPHICS_API_D3D11

		case( unique_index_v< Private::_ConstantBufferImplD3D11, Private::ConstantBufferImpl > ):
		{
			auto& impl         = std::get< Private::_ConstantBufferImplD3D11 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			D3D11_MAPPED_SUBRESOURCE subresource;
			if( context_impl.device_context->Map( impl.buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource ) == S_OK )
				return subresource.pData;

			return nullptr;
		}

	#endif

	}
}

void ConstantBuffer::UpdateEnd()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL

		case( unique_index_v< Private::_ConstantBufferImplOpenGL31, Private::ConstantBufferImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->unmap_buffer( OpenGL::BufferTarget::Uniform );
			context_impl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, 0 );

			break;
		}

	#endif
	#if _ORB_HAS_GRAPHICS_API_D3D11

		case( unique_index_v< Private::_ConstantBufferImplD3D11, Private::ConstantBufferImpl > ):
		{
			auto& impl         = std::get< Private::_ConstantBufferImplD3D11 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.device_context->Unmap( impl.buffer.get(), 0 );

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

		case( unique_index_v< Private::_ConstantBufferImplOpenGL31, Private::ConstantBufferImpl > ):
		{
			auto& impl         = std::get< Private::_ConstantBufferImplOpenGL31 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, impl.id );
			context_impl.functions->bind_buffer_base( OpenGL::BufferTarget::Uniform, slot, impl.id );

			break;
		}

	#endif
	#if _ORB_HAS_GRAPHICS_API_D3D11

		case( unique_index_v< Private::_ConstantBufferImplD3D11, Private::ConstantBufferImpl > ):
		{
			auto&         impl         = std::get< Private::_ConstantBufferImplD3D11 >( m_impl );
			auto&         context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );
			ID3D11Buffer* buffer       = impl.buffer.get();

			switch( type )
			{
				default: break;
				case ShaderType::Vertex:   { context_impl.device_context->VSSetConstantBuffers( slot, 1, &buffer ); } break;
				case ShaderType::Fragment: { context_impl.device_context->PSSetConstantBuffers( slot, 1, &buffer ); } break;
			}

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
