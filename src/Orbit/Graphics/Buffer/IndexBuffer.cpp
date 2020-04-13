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

#include "IndexBuffer.h"

#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Context/RenderContext.h"

ORB_NAMESPACE_BEGIN

static size_t GetFormatSize( IndexFormat format )
{
	switch( format )
	{
		case IndexFormat::Byte:       return 1;
		case IndexFormat::Word:       return 2;
		case IndexFormat::DoubleWord: return 4;
		default:                      return 0;
	}
}

IndexBuffer::IndexBuffer( IndexFormat format, const void* data, size_t count )
	: details_{ }
	, format_ { format }
	, count_  { count }
{
	auto&        context_details = RenderContext::GetInstance().GetPrivateDetails();
	const size_t total_size      = ( count * GetFormatSize( format ) );

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& details = details_.emplace< Private::_IndexBufferDetailsOpenGL >();

			glGenBuffers( 1, &details.id );
			glBindBuffer( OpenGLBufferTarget::ElementArray, details.id );
			glBufferData( OpenGLBufferTarget::ElementArray, total_size, data, OpenGLBufferUsage::StaticDraw );
			glBindBuffer( OpenGLBufferTarget::ElementArray, 0 );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = details_.emplace< Private::_IndexBufferDetailsD3D11 >();
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth      = static_cast< UINT >( ( total_size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage          = D3D11_USAGE_DEFAULT;
			desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
			desc.CPUAccessFlags = 0;

			if( data )
			{
				D3D11_SUBRESOURCE_DATA initial_data { };
				initial_data.pSysMem = data;
				d3d11.device->CreateBuffer( &desc, &initial_data, &details.buffer.ptr_ );
			}
			else
			{
				d3d11.device->CreateBuffer( &desc, nullptr, &details.buffer.ptr_ );
			}

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

IndexBuffer::~IndexBuffer( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_IndexBufferDetailsOpenGL, Private::IndexBufferDetails > ):
		{
			auto& details = std::get< Private::_IndexBufferDetailsOpenGL >( details_ );

			glDeleteBuffers( 1, &details.id );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_IndexBufferDetailsD3D11, Private::IndexBufferDetails > ):
		{
			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

void IndexBuffer::Bind( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_IndexBufferDetailsOpenGL, Private::IndexBufferDetails > ):
		{
			auto& details = std::get< Private::_IndexBufferDetailsOpenGL >( details_ );

			glBindBuffer( OpenGLBufferTarget::ElementArray, details.id );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_IndexBufferDetailsD3D11, Private::IndexBufferDetails > ):
		{
			auto& details = std::get< Private::_IndexBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			/* Translate format to DXGI_FORMAT */
			DXGI_FORMAT format;
			switch( format_ )
			{
				default:                      { format = DXGI_FORMAT_UNKNOWN;  } break;
				case IndexFormat::Byte:       { format = DXGI_FORMAT_R8_UINT;  } break;
				case IndexFormat::Word:       { format = DXGI_FORMAT_R16_UINT; } break;
				case IndexFormat::DoubleWord: { format = DXGI_FORMAT_R32_UINT; } break;
			}

			d3d11.device_context->IASetIndexBuffer( details.buffer.ptr_, format, 0 );

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

const void* IndexBuffer::MapRead( void )
{
	switch( details_.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_IndexBufferDetailsOpenGL, Private::IndexBufferDetails > ):
		{
			auto& details = std::get< Private::_IndexBufferDetailsOpenGL >( details_ );

			glBindBuffer( OpenGLBufferTarget::ElementArray, details.id );
			return glMapBufferRange( OpenGLBufferTarget::ElementArray, 0, ( GetFormatSize( format_ ) * count_ ), OpenGLMapAccess::ReadBit );

		} break;

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_IndexBufferDetailsD3D11, Private::IndexBufferDetails > ):
		{
			auto& details = std::get< Private::_IndexBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			if( d3d11.device_context->Map( details.buffer.ptr_, 0, D3D11_MAP_READ, 0, &mapped_subresource ) == S_OK )
				return mapped_subresource.pData;

		} break;

	#endif // ORB_HAS_D3D11

	}

	return nullptr;
}

void IndexBuffer::Unmap( void )
{
	switch( details_.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_IndexBufferDetailsOpenGL, Private::IndexBufferDetails > ):
		{
			glUnmapBuffer( OpenGLBufferTarget::ElementArray );
			glBindBuffer( OpenGLBufferTarget::ElementArray, 0 );

		} break;

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_IndexBufferDetailsD3D11, Private::IndexBufferDetails > ):
		{
			auto& details = std::get< Private::_IndexBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			d3d11.device_context->Unmap( details.buffer.ptr_, 0 );

		} break;

	#endif // ORB_HAS_D3D11

	}
}

ORB_NAMESPACE_END
