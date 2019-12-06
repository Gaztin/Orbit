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

#include "IndexBuffer.h"

#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/Device/RenderContext.h"

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

IndexBuffer::IndexBuffer( IndexFormat format, const void* index_data, size_t count )
	: m_data   { }
	, m_format { format }
	, m_count  { count }
{
	auto&        context_data = RenderContext::GetInstance().GetPrivateData();
	const size_t total_size   = ( count * GetFormatSize( format ) );

	switch( context_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{
			auto& data = m_data.emplace< Private::_IndexBufferDataOpenGL >();
			auto& gl   = std::get< Private::_RenderContextDataOpenGL >( context_data );

			gl.functions->gen_buffers( 1, &data.id );
			gl.functions->bind_buffer( OpenGL::BufferTarget::ElementArray, data.id );
			gl.functions->buffer_data( OpenGL::BufferTarget::ElementArray, total_size, index_data, OpenGL::BufferUsage::StaticDraw );
			gl.functions->bind_buffer( OpenGL::BufferTarget::ElementArray, 0 );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
		{
			auto& data  = m_data.emplace< Private::_IndexBufferDataD3D11 >();
			auto& d3d11 = std::get< Private::_RenderContextDataD3D11 >( context_data );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth = static_cast< UINT >( ( total_size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage     = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

			ID3D11Buffer* buffer;
			if( index_data )
			{
				D3D11_SUBRESOURCE_DATA initial_data { };
				initial_data.pSysMem = index_data;
				d3d11.device->CreateBuffer( &desc, &initial_data, &buffer );
			}
			else
			{
				d3d11.device->CreateBuffer( &desc, nullptr, &buffer );
			}

			data.buffer.reset( buffer );

			break;
		}

	#endif

	}
}

IndexBuffer::~IndexBuffer( void )
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_IndexBufferDataOpenGL, Private::IndexBufferData > ):
		{
			auto& data = std::get< Private::_IndexBufferDataOpenGL >( m_data );
			auto& gl   = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			gl.functions->delete_buffers( 1, &data.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_IndexBufferDataD3D11, Private::IndexBufferData > ):
		{
			break;
		}

	#endif

	}
}

void IndexBuffer::Bind( void )
{
	switch( m_data.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_IndexBufferDataOpenGL, Private::IndexBufferData > ):
		{
			auto& data = std::get< Private::_IndexBufferDataOpenGL >( m_data );
			auto& gl   = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			gl.functions->bind_buffer( OpenGL::BufferTarget::ElementArray, data.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_IndexBufferDataD3D11, Private::IndexBufferData > ):
		{
			auto& data  = std::get< Private::_IndexBufferDataD3D11 >( m_data );
			auto& d3d11 = std::get< Private::_RenderContextDataD3D11 >( RenderContext::GetInstance().GetPrivateData() );

			/* Translate format to DXGI_FORMAT */
			DXGI_FORMAT format;
			switch( m_format )
			{
				default:                      { format = DXGI_FORMAT_UNKNOWN;  } break;
				case IndexFormat::Byte:       { format = DXGI_FORMAT_R8_UINT;  } break;
				case IndexFormat::Word:       { format = DXGI_FORMAT_R16_UINT; } break;
				case IndexFormat::DoubleWord: { format = DXGI_FORMAT_R32_UINT; } break;
			}

			d3d11.device_context->IASetIndexBuffer( data.buffer.get(), format, 0 );

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
