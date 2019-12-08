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
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Device/RenderContext.h"

ORB_NAMESPACE_BEGIN

ConstantBuffer::ConstantBuffer( size_t size )
{
	auto& context_details = RenderContext::GetInstance().GetPrivateDetails();

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& gl = std::get< Private::_RenderContextDetailsOpenGL >( context_details );

			if( (  gl.embedded && gl.opengl_version < Version( 3 ) )||
			    ( !gl.embedded && gl.opengl_version < Version( 3, 1 ) ) )
			{
				m_details.emplace< Private::_ConstantBufferDetailsOpenGL20 >();
			}
			else
			{
				auto& details = m_details.emplace< Private::_ConstantBufferDetailsOpenGL31 >();

				glGenBuffers( 1, &details.id );
				glBindBuffer( OpenGLBufferTarget::Uniform, details.id );
				glBufferData( OpenGLBufferTarget::Uniform, size, nullptr, OpenGLBufferUsage::StreamDraw );
				glBindBuffer( OpenGLBufferTarget::Uniform, 0 );
			}
			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = m_details.emplace< Private::_ConstantBufferDetailsD3D11 >();
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth      = static_cast< UINT >( ( size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			ID3D11Buffer* buffer;
			d3d11.device->CreateBuffer( &desc, nullptr, &buffer );
			details.buffer.reset( buffer );

			break;
		}

	#endif

	}
}

ConstantBuffer::~ConstantBuffer( void )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsOpenGL31 >( m_details );

			glDeleteBuffers( 1, &details.id );

			break;
		}

	#endif

	}
}

void ConstantBuffer::Update( void* dst, size_t location, const void* data, size_t size )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL20, Private::ConstantBufferDetails > ):
		{
			glUniform1f( static_cast< GLint >( location ), *reinterpret_cast< const GLfloat* >( data ) );

			break;
		}

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			std::memcpy( dst, data, size );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDetailsD3D11, Private::ConstantBufferDetails > ):
		{
			std::memcpy( dst, data, size );

			break;
		}

	#endif

	}
}

void* ConstantBuffer::UpdateBegin( size_t size )
{
	switch( m_details.index() )
	{
		default: return nullptr;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsOpenGL31 >( m_details );

			glBindBuffer( OpenGLBufferTarget::Uniform, details.id );
			return glMapBufferRange( OpenGLBufferTarget::Uniform, 0, size, OpenGLMapAccess::WriteBit );
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDetailsD3D11, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsD3D11 >( m_details );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			D3D11_MAPPED_SUBRESOURCE subresource;
			if( d3d11.device_context->Map( details.buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource ) == S_OK )
				return subresource.pData;

			return nullptr;
		}

	#endif

	}
}

void ConstantBuffer::UpdateEnd()
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			glUnmapBuffer( OpenGLBufferTarget::Uniform );
			glBindBuffer( OpenGLBufferTarget::Uniform, 0 );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDetailsD3D11, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsD3D11 >( m_details );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			d3d11.device_context->Unmap( details.buffer.get(), 0 );

			break;
		}

	#endif

	}
}

void ConstantBuffer::Bind( [[ maybe_unused ]] ShaderType type, uint32_t slot )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsOpenGL31 >( m_details );

			glBindBuffer( OpenGLBufferTarget::Uniform, details.id );
			glBindBufferBase( OpenGLBufferTarget::Uniform, slot, details.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDetailsD3D11, Private::ConstantBufferDetails > ):
		{
			auto&         details = std::get< Private::_ConstantBufferDetailsD3D11 >( m_details );
			auto&         d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			ID3D11Buffer* buffer  = details.buffer.get();

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
