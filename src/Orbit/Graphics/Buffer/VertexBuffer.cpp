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
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Context/RenderContext.h"

ORB_NAMESPACE_BEGIN

VertexBuffer::VertexBuffer( const void* data, size_t count, size_t stride )
	: m_details { }
	, m_count   { count }
{
	auto&        context_details = RenderContext::Get().GetPrivateDetails();
	const size_t total_size      = ( count * stride );

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& details = m_details.emplace< Private::_VertexBufferDetailsOpenGL >();

			glGenBuffers( 1, &details.id );
			glBindBuffer( OpenGLBufferTarget::Array, details.id );
			glBufferData( OpenGLBufferTarget::Array, total_size, data, OpenGLBufferUsage::StaticDraw );
			glBindBuffer( OpenGLBufferTarget::Array, 0 );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = m_details.emplace< Private::_VertexBufferDetailsD3D11 >();
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth = static_cast< UINT >( ( total_size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage     = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			ID3D11Buffer* buffer;
			if( data )
			{
				D3D11_SUBRESOURCE_DATA initial_data { };
				initial_data.pSysMem = data;
				d3d11.device->CreateBuffer( &desc, &initial_data, &buffer );
			}
			else
			{
				d3d11.device->CreateBuffer( &desc, nullptr, &buffer );
			}

			details.buffer.reset( buffer );
			details.stride = static_cast< UINT >( stride );

			break;
		}

	#endif

	}
}

VertexBuffer::~VertexBuffer( void )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsOpenGL >( m_details );

			glDeleteBuffers( 1, &details.id );

			break;
		}

	#endif

	}
}

void VertexBuffer::Bind( void )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto& details = std::get< Private::_VertexBufferDetailsOpenGL >( m_details );

			glBindBuffer( OpenGLBufferTarget::Array, details.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_VertexBufferDetailsD3D11, Private::VertexBufferDetails > ):
		{
			auto&         details = std::get< Private::_VertexBufferDetailsD3D11 >( m_details );
			auto&         d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::Get().GetPrivateDetails() );
			ID3D11Buffer* buffer  = details.buffer.get();
			UINT          stride  = details.stride;
			UINT          offset  = 0;

			d3d11.device_context->IASetVertexBuffers( 0, 1, &buffer, &stride, &offset );

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
