/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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

#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLVersion.h"
#include "Orbit/Graphics/Context/RenderContext.h"

#include <cstring>

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

			if( gl.version.RequireGL( 3, 1 ) || gl.version.RequireGLES( 3 ) )
			{
				auto& details = details_.emplace< Private::_ConstantBufferDetailsOpenGL31 >();

				glGenBuffers( 1, &details.id );
				glBindBuffer( OpenGLBufferTarget::Uniform, details.id );
				glBufferData( OpenGLBufferTarget::Uniform, size, nullptr, OpenGLBufferUsage::StreamDraw );
				glBindBuffer( OpenGLBufferTarget::Uniform, 0 );
			}
			else
			{
				details_.emplace< Private::_ConstantBufferDetailsOpenGL20 >();
			}

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = details_.emplace< Private::_ConstantBufferDetailsD3D11 >();
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth      = static_cast< UINT >( ( size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage          = D3D11_USAGE_DYNAMIC;
			desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			d3d11.device->CreateBuffer( &desc, nullptr, &details.buffer.ptr_ );

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

ConstantBuffer::~ConstantBuffer( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsOpenGL31 >( details_ );

			glDeleteBuffers( 1, &details.id );

			break;
		}

	#endif // ORB_HAS_OPENGL

	}
}

void ConstantBuffer::Bind( ShaderType type, uint32_t local_slot, uint32_t global_slot ) const
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsOpenGL31 >( details_ );

			glBindBuffer( OpenGLBufferTarget::Uniform, details.id );
			glBindBufferBase( OpenGLBufferTarget::Uniform, global_slot, details.id );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDetailsD3D11, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			switch( type )
			{
				default: break;
				case ShaderType::Vertex:   { d3d11.device_context->VSSetConstantBuffers( local_slot, 1, &details.buffer.ptr_ ); } break;
				case ShaderType::Fragment: { d3d11.device_context->PSSetConstantBuffers( local_slot, 1, &details.buffer.ptr_ ); } break;
			}

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

void ConstantBuffer::Unbind( ShaderType type, uint32_t local_slot, uint32_t global_slot ) const
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			glBindBufferBase( OpenGLBufferTarget::Uniform, global_slot, 0 );
			glBindBuffer( OpenGLBufferTarget::Uniform, 0 );

		} break;

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDetailsD3D11, Private::ConstantBufferDetails > ):
		{
			auto&         d3d11       = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			ID3D11Buffer* null_buffer = nullptr;

			switch( type )
			{
				default: break;
				case ShaderType::Vertex:   { d3d11.device_context->VSSetConstantBuffers( local_slot, 1, &null_buffer ); } break;
				case ShaderType::Fragment: { d3d11.device_context->PSSetConstantBuffers( local_slot, 1, &null_buffer ); } break;
			}

		} break;

	#endif // ORB_HAS_D3D11

	}
}

void ConstantBuffer::Update( const void* data, size_t size )
{
	void* dst = UpdateBegin( size );

	std::memcpy( dst, data, size );

	UpdateEnd();
}

void* ConstantBuffer::UpdateBegin( size_t size )
{
	switch( details_.index() )
	{
		default: return nullptr;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsOpenGL31 >( details_ );

			glBindBuffer( OpenGLBufferTarget::Uniform, details.id );
			return glMapBufferRange( OpenGLBufferTarget::Uniform, 0, size, OpenGLMapAccess::WriteBit );
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDetailsD3D11, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			D3D11_MAPPED_SUBRESOURCE subresource;
			if( d3d11.device_context->Map( details.buffer.ptr_, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource ) == S_OK )
				return subresource.pData;

			return nullptr;
		}

	#endif // ORB_HAS_D3D11

	}
}

void ConstantBuffer::UpdateEnd()
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ConstantBufferDetailsOpenGL31, Private::ConstantBufferDetails > ):
		{
			glUnmapBuffer( OpenGLBufferTarget::Uniform );
			glBindBuffer( OpenGLBufferTarget::Uniform, 0 );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ConstantBufferDetailsD3D11, Private::ConstantBufferDetails > ):
		{
			auto& details = std::get< Private::_ConstantBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			d3d11.device_context->Unmap( details.buffer.ptr_, 0 );

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

ORB_NAMESPACE_END
