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

IndexBuffer::IndexBuffer( IndexFormat format, const void* data, size_t count )
	: m_impl   { }
	, m_format { format }
	, m_count  { count }
{
	auto&        context_impl_var = RenderContext::GetInstance().GetPrivateData();
	const size_t total_size       = ( count * GetFormatSize( format ) );

	switch( context_impl_var.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{
			auto& impl         = m_impl.emplace< Private::_IndexBufferDataOpenGL >();
			auto& context_impl = std::get< Private::_RenderContextDataOpenGL >( context_impl_var );

			context_impl.functions->gen_buffers( 1, &impl.id );
			context_impl.functions->bind_buffer( OpenGL::BufferTarget::ElementArray, impl.id );
			context_impl.functions->buffer_data( OpenGL::BufferTarget::ElementArray, total_size, data, OpenGL::BufferUsage::StaticDraw );
			context_impl.functions->bind_buffer( OpenGL::BufferTarget::ElementArray, 0 );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
		{
			auto& impl         = m_impl.emplace< Private::_IndexBufferDataD3D11 >();
			auto& context_impl = std::get< Private::_RenderContextDataD3D11 >( context_impl_var );

			D3D11_BUFFER_DESC desc { };
			desc.ByteWidth = static_cast< UINT >( ( total_size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage     = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

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

			break;
		}

	#endif

	}
}

IndexBuffer::~IndexBuffer( void )
{
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_IndexBufferDataOpenGL, Private::IndexBufferData > ):
		{
			auto& impl         = std::get< Private::_IndexBufferDataOpenGL >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			context_impl.functions->delete_buffers( 1, &impl.id );

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
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_IndexBufferDataOpenGL, Private::IndexBufferData > ):
		{
			auto& impl         = std::get< Private::_IndexBufferDataOpenGL >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );

			context_impl.functions->bind_buffer( OpenGL::BufferTarget::ElementArray, impl.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_IndexBufferDataD3D11, Private::IndexBufferData > ):
		{
			auto& impl         = std::get< Private::_IndexBufferDataD3D11 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextDataD3D11 >( RenderContext::GetInstance().GetPrivateData() );

			/* Translate format to DXGI_FORMAT */
			DXGI_FORMAT format;
			switch( m_format )
			{
				default:                      { format = DXGI_FORMAT_UNKNOWN;  } break;
				case IndexFormat::Byte:       { format = DXGI_FORMAT_R8_UINT;  } break;
				case IndexFormat::Word:       { format = DXGI_FORMAT_R16_UINT; } break;
				case IndexFormat::DoubleWord: { format = DXGI_FORMAT_R32_UINT; } break;
			}

			context_impl.device_context->IASetIndexBuffer( impl.buffer.get(), format, 0 );

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
