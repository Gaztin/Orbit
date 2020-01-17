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

#include "Texture2D.h"

#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Context/RenderContext.h"

ORB_NAMESPACE_BEGIN

Texture2D::Texture2D( uint32_t width, uint32_t height, const void* data )
{
	auto& context_details = RenderContext::Get().GetPrivateDetails();

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& details = m_details.emplace< Private::_Texture2DDetailsOpenGL >();

			glGenTextures( 1, &details.id );
			glBindTexture( GL_TEXTURE_2D, details.id );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			if( data )
			{
				glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
			}

			glBindTexture( GL_TEXTURE_2D, 0 );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = m_details.emplace< Private::_Texture2DDetailsD3D11 >();
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			D3D11_TEXTURE2D_DESC texture2d_desc { };
			texture2d_desc.Width              = width;
			texture2d_desc.Height             = height;
			texture2d_desc.MipLevels          = 1;
			texture2d_desc.ArraySize          = 1;
			texture2d_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
			texture2d_desc.SampleDesc.Count   = 1;
			texture2d_desc.SampleDesc.Quality = 0;
			texture2d_desc.Usage              = D3D11_USAGE_DEFAULT;
			texture2d_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
			texture2d_desc.CPUAccessFlags     = 0;

			if( data )
			{
				D3D11_SUBRESOURCE_DATA initial_data { };
				initial_data.pSysMem     = data;
				initial_data.SysMemPitch = width * 4;

				ID3D11Texture2D* texture_2d;
				if( d3d11.device->CreateTexture2D( &texture2d_desc, &initial_data, &texture_2d ) == S_OK )
				{
					details.texture_2d.reset( texture_2d );
				}
			}
			else
			{
				ID3D11Texture2D* texture_2d;
				if( d3d11.device->CreateTexture2D( &texture2d_desc, nullptr, &texture_2d ) == S_OK )
				{
					details.texture_2d.reset( texture_2d );
				}
			}

			if( details.texture_2d )
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc { };
				srv_desc.Format              = texture2d_desc.Format;
				srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = 1;

				ID3D11ShaderResourceView* srv;
				if( d3d11.device->CreateShaderResourceView( details.texture_2d.get(), &srv_desc, &srv ) == S_OK )
				{
					details.shader_resource_view.reset( srv );
				}

				break;
			}
		}
	#endif

	}
}

Texture2D::~Texture2D( void )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_Texture2DDetailsOpenGL, Private::Texture2DDetails > ):
		{
			auto& details = std::get< Private::_Texture2DDetailsOpenGL >( m_details );
			
			glDeleteTextures( 1, &details.id );

			break;
		}

	#endif

	}
}

void Texture2D::Bind( uint32_t slot )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_Texture2DDetailsOpenGL, Private::Texture2DDetails > ):
		{
			auto&          details   = std::get< Private::_Texture2DDetailsOpenGL >( m_details );
			const uint32_t unit_base = static_cast< GLenum >( OpenGLTextureUnit::Texture0 );

			glActiveTexture( static_cast< OpenGLTextureUnit >( unit_base + slot ) );
			glBindTexture( GL_TEXTURE_2D, details.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_Texture2DDetailsD3D11, Private::Texture2DDetails > ):
		{
			auto&                     details = std::get< Private::_Texture2DDetailsD3D11 >( m_details );
			auto&                     d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::Get().GetPrivateDetails() );
			ID3D11ShaderResourceView* srv     = details.shader_resource_view.get();

			d3d11.device_context->PSSetShaderResources( slot, 1, &srv );

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
