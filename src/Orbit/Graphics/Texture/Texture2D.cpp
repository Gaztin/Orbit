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

#include "Texture2D.h"

#include "Orbit/Core/Platform/Windows/Win32Error.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Context/RenderContext.h"

ORB_NAMESPACE_BEGIN

#if( ORB_HAS_OPENGL )

static GLuint PixelFormatToGLFormat( PixelFormat pixel_format )
{
	switch( pixel_format )
	{
		case PixelFormat::R:    return GL_LUMINANCE;
		case PixelFormat::RGBA: return GL_RGBA;
		default:                return 0;
	}
}

#endif // ORB_HAS_OPENGL
#if( ORB_HAS_D3D11 )

static DXGI_FORMAT PixelFormatToDXGIFormat( PixelFormat pixel_format )
{
	switch( pixel_format )
	{
		case Orbit::PixelFormat::R:    return DXGI_FORMAT_R8_UNORM;
		case Orbit::PixelFormat::RGBA: return DXGI_FORMAT_R8G8B8A8_UNORM;
		default:                       return DXGI_FORMAT_UNKNOWN;
	}
}

#endif // ORB_HAS_D3D11

Texture2D::Texture2D( uint32_t width, uint32_t height, const void* data, PixelFormat pixel_format )
{
	auto& context_details = RenderContext::GetInstance().GetPrivateDetails();

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& details = details_.emplace< Private::_Texture2DDetailsOpenGL >();

			glGenTextures( 1, &details.id );
			glBindTexture( GL_TEXTURE_2D, details.id );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			if( data )
			{
				const GLenum format = PixelFormatToGLFormat( pixel_format );

				glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data );
			}

			glBindTexture( GL_TEXTURE_2D, 0 );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& details = details_.emplace< Private::_Texture2DDetailsD3D11 >();
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			D3D11_TEXTURE2D_DESC texture2d_desc { };
			texture2d_desc.Width              = width;
			texture2d_desc.Height             = height;
			texture2d_desc.MipLevels          = 1;
			texture2d_desc.ArraySize          = 1;
			texture2d_desc.Format             = PixelFormatToDXGIFormat( pixel_format );
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

				ORB_CHECK_HRESULT( d3d11.device->CreateTexture2D( &texture2d_desc, &initial_data, &details.texture_2d.ptr_ ) );
			}
			else
			{
				ORB_CHECK_HRESULT( d3d11.device->CreateTexture2D( &texture2d_desc, nullptr, &details.texture_2d.ptr_ ) );
			}

			if( details.texture_2d )
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc { };
				srv_desc.Format              = texture2d_desc.Format;
				srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = 1;

				ORB_CHECK_HRESULT( d3d11.device->CreateShaderResourceView( details.texture_2d.ptr_, &srv_desc, &details.shader_resource_view.ptr_ ) );
			}
		}
	#endif // ORB_HAS_D3D11

	}
}

Texture2D::~Texture2D( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_Texture2DDetailsOpenGL, Private::Texture2DDetails > ):
		{
			auto& details = std::get< Private::_Texture2DDetailsOpenGL >( details_ );
			
			glDeleteTextures( 1, &details.id );

			break;
		}

	#endif // ORB_HAS_OPENGL

	}
}

void Texture2D::Bind( uint32_t slot )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_Texture2DDetailsOpenGL, Private::Texture2DDetails > ):
		{
			auto&          details   = std::get< Private::_Texture2DDetailsOpenGL >( details_ );
			const uint32_t unit_base = static_cast< GLenum >( OpenGLTextureUnit::Texture0 );

			glActiveTexture( static_cast< OpenGLTextureUnit >( unit_base + slot ) );
			glBindTexture( GL_TEXTURE_2D, details.id );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_Texture2DDetailsD3D11, Private::Texture2DDetails > ):
		{
			auto& details = std::get< Private::_Texture2DDetailsD3D11 >( details_ );
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			d3d11.device_context->PSSetShaderResources( slot, 1, &details.shader_resource_view.ptr_ );

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

void Texture2D::Unbind( uint32_t slot )
{
	switch( details_.index() )
	{
		default: break;

		#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_Texture2DDetailsOpenGL, Private::Texture2DDetails > ):
		{
			const uint32_t unit_base = static_cast< GLenum >( OpenGLTextureUnit::Texture0 );

			glActiveTexture( static_cast< OpenGLTextureUnit >( unit_base + slot ) );
			glBindTexture( GL_TEXTURE_2D, 0 );

			break;
		}

	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_Texture2DDetailsD3D11, Private::Texture2DDetails > ):
		{
			auto& d3d11 = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			ID3D11ShaderResourceView* null_view = nullptr;
			d3d11.device_context->PSSetShaderResources( slot, 1, &null_view );

			break;
		}

	#endif // ORB_HAS_D3D11

	}
}

ORB_NAMESPACE_END
