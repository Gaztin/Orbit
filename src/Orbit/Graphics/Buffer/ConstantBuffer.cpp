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
	auto& context_data = RenderContext::GetInstance().GetPrivateData();

	switch( context_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{
			auto& gl = std::get< Private::_RenderContextDataOpenGL >( context_data );

			if( (  gl.embedded && gl.opengl_version < Version( 3 ) )||
			    ( !gl.embedded && gl.opengl_version < Version( 3, 1 ) ) )
			{
				m_data.emplace< Private::_ConstantBufferDataOpenGL20 >();
			}
			else
			{
				auto& data = m_data.emplace< Private::_ConstantBufferDataOpenGL31 >();

				gl.functions->gen_buffers( 1, &data.id );
				gl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, data.id );
				gl.functions->buffer_data( OpenGL::BufferTarget::Uniform, size, nullptr, OpenGL::BufferUsage::StreamDraw );
				gl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, 0 );
			}
			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
		{
			auto& data  = m_data.emplace< Private::_ConstantBufferDataD3D11 >();
			auto& d3d11 = std::get< Private::_RenderContextDataD3D11 >( context_data );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth      = static_cast< UINT >( ( size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			ID3D11Buffer* buffer;
			d3d11.device->CreateBuffer( &desc, nullptr, &buffer );
			data.buffer.reset( buffer );

			break;
		}

	#endif

	}
}

ConstantBuffer::~ConstantBuffer( void )
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDataOpenGL31, Private::ConstantBufferData > ):
		{
			auto& data = std::get< Private::_ConstantBufferDataOpenGL31 >( m_data );
			auto& gl   = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			gl.functions->delete_buffers( 1, &data.id );

			break;
		}

	#endif

	}
}

void ConstantBuffer::Update( void* dst, size_t location, const void* data, size_t size )
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDataOpenGL20, Private::ConstantBufferData > ):
		{
			auto& gl = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			gl.functions->uniform1f( static_cast< GLint >( location ), *reinterpret_cast< const GLfloat* >( data ) );

			break;
		}

		case( unique_index_v< Private::_ConstantBufferDataOpenGL31, Private::ConstantBufferData > ):
		{
			std::memcpy( dst, data, size );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDataD3D11, Private::ConstantBufferData > ):
		{
			std::memcpy( dst, data, size );

			break;
		}

	#endif

	}
}

void* ConstantBuffer::UpdateBegin( size_t size )
{
	switch( m_data.index() )
	{
		default: return nullptr;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDataOpenGL31, Private::ConstantBufferData > ):
		{
			auto& data = std::get< Private::_ConstantBufferDataOpenGL31 >( m_data );
			auto& gl   = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			gl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, data.id );
			return gl.functions->map_buffer_range( OpenGL::BufferTarget::Uniform, 0, size, OpenGL::MapAccess::WriteBit );
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDataD3D11, Private::ConstantBufferData > ):
		{
			auto& data  = std::get< Private::_ConstantBufferDataD3D11 >( m_data );
			auto& d3d11 = std::get< Private::_RenderContextDataD3D11 >( RenderContext::GetInstance().GetPrivateData() );

			D3D11_MAPPED_SUBRESOURCE subresource;
			if( d3d11.device_context->Map( data.buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource ) == S_OK )
				return subresource.pData;

			return nullptr;
		}

	#endif

	}
}

void ConstantBuffer::UpdateEnd()
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDataOpenGL31, Private::ConstantBufferData > ):
		{
			auto& gl = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			gl.functions->unmap_buffer( OpenGL::BufferTarget::Uniform );
			gl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, 0 );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDataD3D11, Private::ConstantBufferData > ):
		{
			auto& data  = std::get< Private::_ConstantBufferDataD3D11 >( m_data );
			auto& d3d11 = std::get< Private::_RenderContextDataD3D11 >( RenderContext::GetInstance().GetPrivateData() );

			d3d11.device_context->Unmap( data.buffer.get(), 0 );

			break;
		}

	#endif

	}
}

void ConstantBuffer::Bind( ShaderType type, uint32_t slot )
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDataOpenGL31, Private::ConstantBufferData > ):
		{
			auto& data = std::get< Private::_ConstantBufferDataOpenGL31 >( m_data );
			auto& gl   = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			gl.functions->bind_buffer( OpenGL::BufferTarget::Uniform, data.id );
			gl.functions->bind_buffer_base( OpenGL::BufferTarget::Uniform, slot, data.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDataD3D11, Private::ConstantBufferData > ):
		{
			auto&         data   = std::get< Private::_ConstantBufferDataD3D11 >( m_data );
			auto&         d3d11  = std::get< Private::_RenderContextDataD3D11 >( RenderContext::GetInstance().GetPrivateData() );
			ID3D11Buffer* buffer = data.buffer.get();

			switch( type )
			{
				default: break;
				case ShaderType::Vertex:   { d3d11.device_context->VSSetConstantBuffers( slot, 1, &buffer ); } break;
				case ShaderType::Fragment: { d3d11.device_context->PSSetConstantBuffers( slot, 1, &buffer ); } break;
			}

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
