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

#include "Orbit/Graphics/Device/RenderContext.h"

ORB_NAMESPACE_BEGIN

Texture2D::Texture2D( uint32_t width, uint32_t height, const void* data )
{
	auto& context_impl_var = RenderContext::GetInstance().GetPrivateData();

	switch( context_impl_var.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
		{
			auto& impl = m_impl.emplace< Private::_Texture2DDataOpenGL >();

			glGenTextures( 1, &impl.id );
			glBindTexture( GL_TEXTURE_2D, impl.id );
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

		case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
		{
			auto& impl         = m_impl.emplace< Private::_Texture2DDataD3D11 >();
			auto& context_impl = std::get< Private::_RenderContextDataD3D11 >( context_impl_var );

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
				if( context_impl.device->CreateTexture2D( &texture2d_desc, &initial_data, &texture_2d ) == S_OK )
				{
					impl.texture_2d.reset( texture_2d );
				}
			}
			else
			{
				ID3D11Texture2D* texture_2d;
				if( context_impl.device->CreateTexture2D( &texture2d_desc, nullptr, &texture_2d ) == S_OK )
				{
					impl.texture_2d.reset( texture_2d );
				}
			}

			if( impl.texture_2d )
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc { };
				srv_desc.Format              = texture2d_desc.Format;
				srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = 1;

				ID3D11ShaderResourceView* srv;
				if( context_impl.device->CreateShaderResourceView( impl.texture_2d.get(), &srv_desc, &srv ) == S_OK )
				{
					impl.shader_resource_view.reset( srv );
				}

				break;
			}
		}
	#endif

	}
}

Texture2D::~Texture2D( void )
{
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_Texture2DDataOpenGL, Private::Texture2DData > ):
		{
			auto& impl         = std::get< Private::_Texture2DDataOpenGL >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );
			
			context_impl.functions->glDeleteTextures( 1, &impl.id );

			break;
		}
	#endif

	}
}

void Texture2D::Bind( uint32_t slot )
{
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_Texture2DDataOpenGL, Private::Texture2DData > ):
		{
			auto&          impl         = std::get< Private::_Texture2DDataOpenGL >( m_impl );
			auto&          context_impl = std::get< Private::_RenderContextDataOpenGL >( RenderContext::GetInstance().GetPrivateData() );
			const uint32_t unit_base    = static_cast< GLenum >( OpenGL::TextureUnit::Texture0 );

			context_impl.functions->glActiveTexture( static_cast< OpenGL::TextureUnit >( unit_base + slot ) );
			context_impl.functions->glBindTexture( GL_TEXTURE_2D, impl.id );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_Texture2DDataD3D11, Private::Texture2DData > ):
		{
			auto&                     impl         = std::get< Private::_Texture2DDataD3D11 >( m_impl );
			auto&                     context_impl = std::get< Private::_RenderContextDataD3D11 >( RenderContext::GetInstance().GetPrivateData() );
			ID3D11ShaderResourceView* srv          = impl.shader_resource_view.get();

			context_impl.device_context->PSSetShaderResources( slot, 1, &srv );

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
