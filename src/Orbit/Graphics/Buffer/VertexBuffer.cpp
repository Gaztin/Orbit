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

#include "VertexBuffer.h"

#include "Orbit/Core/Platform/Windows/Win32Error.h"
#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Context/RenderContext.h"

ORB_NAMESPACE_BEGIN

VertexBuffer::VertexBuffer( const void* data, size_t count, size_t stride, bool is_static )
	: details_  { }
	, count_    ( count )
	, stride_   ( stride )
	, is_static_( is_static )
{
	auto& context_details = RenderContext::GetInstance().GetPrivateDetails();

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& details = details_.emplace< Private::_VertexBufferDetailsOpenGL >();

			glGenBuffers( 1, &details.id );
			glBindBuffer( OpenGLBufferTarget::Array, details.id );
			glBufferData( OpenGLBufferTarget::Array, GetTotalSize(), data, is_static_ ? OpenGLBufferUsage::StaticDraw : OpenGLBufferUsage::StreamDraw );
			glBindBuffer( OpenGLBufferTarget::Array, 0 );

		} break;

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = details_.emplace< Private::_VertexBufferDetailsD3D11 >();
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			D3D11_BUFFER_DESC desc = { };
			desc.ByteWidth = static_cast< UINT >( ( GetTotalSize() + 0xf ) & ~0xf ); // Align by 16 bytes
			desc.Usage     = is_static_ ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			if( data )
			{
				D3D11_SUBRESOURCE_DATA initial_data = { };
				initial_data.pSysMem = data;

				ORB_CHECK_HRESULT( d3d11.device->CreateBuffer( &desc, &initial_data, &details.buffer.ptr_ ) );
			}
			else
			{
				ORB_CHECK_HRESULT( d3d11.device->CreateBuffer( &desc, nullptr, &details.buffer.ptr_ ) );
			}

		} break;

	#endif // ORB_HAS_D3D11

	}
}

VertexBuffer::~VertexBuffer( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsOpenGL >( details_ );

			glDeleteBuffers( 1, &details.id );

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

void VertexBuffer::Update( const void* data, size_t count )
{
	if( data == nullptr && count < count_ )
		return;

	count_ = count;

	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsOpenGL >( details_ );

			glBindBuffer( OpenGLBufferTarget::Array, details.id );
			glBufferData( OpenGLBufferTarget::Array, GetTotalSize(), data, is_static_ ? OpenGLBufferUsage::StaticDraw : OpenGLBufferUsage::StreamDraw );
			glBindBuffer( OpenGLBufferTarget::Array, 0 );

		} break;

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_VertexBufferDetailsD3D11, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			details.buffer = nullptr;

			D3D11_BUFFER_DESC desc = { };
			desc.ByteWidth = static_cast< UINT >( ( GetTotalSize() + 0xf ) & ~0xf ); // Align by 16 bytes
			desc.Usage     = is_static_ ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			if( data )
			{
				D3D11_SUBRESOURCE_DATA initial_data = { };
				initial_data.pSysMem = data;

				ORB_CHECK_HRESULT( d3d11.device->CreateBuffer( &desc, &initial_data, &details.buffer.ptr_ ) );
			}
			else
			{
				ORB_CHECK_HRESULT( d3d11.device->CreateBuffer( &desc, nullptr, &details.buffer.ptr_ ) );
			}

		} break;

	#endif // ORB_HAS_D3D11

	}
}

void VertexBuffer::Bind( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsOpenGL >( details_ );

			glBindBuffer( OpenGLBufferTarget::Array, details.id );

		} break;

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_VertexBufferDetailsD3D11, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			UINT  stride  = static_cast< UINT >( stride_ );
			UINT  offset  = 0;

			d3d11.device_context->IASetVertexBuffers( 0, 1, &details.buffer.ptr_, &stride, &offset );

		} break;

	#endif // ORB_HAS_D3D11

	}
}

void* VertexBuffer::Map( void )
{
	switch( details_.index() )
	{
		default: return nullptr;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsOpenGL >( details_ );

			glBindBuffer( OpenGLBufferTarget::Array, details.id );
			return glMapBufferRange( OpenGLBufferTarget::Array, 0, GetTotalSize(), OpenGLMapAccess::WriteBit );

		} break;

	#endif // ORB_HAS_OPENGL
	#if ORB_HAS_D3D11

		case( unique_index_v< Private::_VertexBufferDetailsD3D11, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			D3D11_MAPPED_SUBRESOURCE mapped;
			if( ORB_CHECK_HRESULT( d3d11.device_context->Map( details.buffer.ptr_, 0, D3D11_MAP_WRITE, 0, &mapped ) ) )
				return mapped.pData;

			return nullptr;

		} break;

	#endif // ORB_HAS_D3D11

	}
}

void VertexBuffer::Unmap( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsOpenGL >( details_ );

			glUnmapBuffer( OpenGLBufferTarget::Array );
			glBindBuffer( OpenGLBufferTarget::Array, 0 );

		} break;

	#endif // ORB_HAS_OPENGL
	#if ORB_HAS_D3D11

		case( unique_index_v< Private::_VertexBufferDetailsD3D11, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			d3d11.device_context->Unmap( details.buffer.ptr_, 0 );

		} break;

	#endif // ORB_HAS_D3D11

	}
}

ORB_NAMESPACE_END
