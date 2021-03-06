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

#include "FrameBuffer.h"

#include "Orbit/Core/Platform/Windows/Win32Error.h"
#include "Orbit/Core/Widget/Window.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Context/RenderContext.h"

ORB_NAMESPACE_BEGIN

FrameBuffer::FrameBuffer( void )
{
	auto& context = RenderContext::GetInstance().GetPrivateDetails();

	// Resize event
	on_resize_ = Window::GetInstance().Subscribe(
		[ this ]( const WindowResized& e )
		{
			Resize( e.width, e.height );
		}
	);

	switch( context.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			framebuffer_details_.emplace< Private::_FrameBufferDetailsD3D11 >();

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& details = framebuffer_details_.emplace< Private::_FrameBufferDetailsOpenGL >();

			// Get new handles
			glGenFramebuffers( 1, &details.fbo );
			glGenRenderbuffers( 1, &details.rbo );

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

FrameBuffer::~FrameBuffer( void )
{

#if( ORB_HAS_OPENGL )

	if( framebuffer_details_.index() == unique_index_v< Private::_FrameBufferDetailsOpenGL, Private::FrameBufferDetails > )
	{
		auto& details = std::get< Private::_FrameBufferDetailsOpenGL >( framebuffer_details_ );

		glDeleteFramebuffers( 1, &details.fbo );
	}

#endif // ORB_HAS_OPENGL

}

void FrameBuffer::Clear( void )
{
	switch( framebuffer_details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_FrameBufferDetailsD3D11, Private::FrameBufferDetails > ):
		{
			auto&       d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto&       details = std::get< Private::_FrameBufferDetailsD3D11 >( framebuffer_details_ );
			const FLOAT clear_color[ 4 ]{ };

			d3d11.device_context->ClearRenderTargetView( details.render_target_view.ptr_, clear_color );

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_FrameBufferDetailsOpenGL, Private::FrameBufferDetails > ):
		{
			auto& details = std::get< Private::_FrameBufferDetailsOpenGL >( framebuffer_details_ );

			glBindFramebuffer( OpenGLFramebufferTarget::Draw, details.fbo );
			glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
			glBindFramebuffer( OpenGLFramebufferTarget::Both, 0 );

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

void FrameBuffer::Bind( void )
{
	switch( framebuffer_details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_FrameBufferDetailsD3D11, Private::FrameBufferDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details = std::get< Private::_FrameBufferDetailsD3D11 >( framebuffer_details_ );

			ORB_CHECK_SYSTEM_ERROR( d3d11.device_context->OMSetRenderTargets( 1, &details.render_target_view.ptr_, d3d11.depth_stencil_view.ptr_ ) );

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_FrameBufferDetailsOpenGL, Private::FrameBufferDetails > ):
		{
			auto& details = std::get< Private::_FrameBufferDetailsOpenGL >( framebuffer_details_ );

			glBindFramebuffer( OpenGLFramebufferTarget::Draw, details.fbo );

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

void FrameBuffer::Unbind( void )
{
	switch( framebuffer_details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_FrameBufferDetailsD3D11, Private::FrameBufferDetails > ):
		{
			auto& d3d11 = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );

			/* Re-bind back buffer */
			ORB_CHECK_SYSTEM_ERROR( d3d11.device_context->OMSetRenderTargets( 1, &d3d11.render_target_view.ptr_, d3d11.depth_stencil_view.ptr_ ) );

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_FrameBufferDetailsOpenGL, Private::FrameBufferDetails > ):
		{
			glBindFramebuffer( OpenGLFramebufferTarget::Draw, 0 );

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

void FrameBuffer::Resize( uint32_t width, uint32_t height )
{
	switch( framebuffer_details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_FrameBufferDetailsD3D11, Private::FrameBufferDetails > ):
		{
			auto& d3d11     = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details   = std::get< Private::_FrameBufferDetailsD3D11 >( framebuffer_details_ );
			auto& texture2d = std::get< Private::_Texture2DDetailsD3D11 >( texture2d_.GetPrivateDetails() );

			// Release objects
			details.render_target_view     = nullptr;
			texture2d.texture2d            = nullptr;
			texture2d.shader_resource_view = nullptr;

			// Create texture
			D3D11_TEXTURE2D_DESC texture2d_desc;
			texture2d_desc.Width              = width;
			texture2d_desc.Height             = height;
			texture2d_desc.MipLevels          = 0;
			texture2d_desc.ArraySize          = 1;
			texture2d_desc.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
			texture2d_desc.SampleDesc.Count   = 1;
			texture2d_desc.SampleDesc.Quality = 0;
			texture2d_desc.Usage              = D3D11_USAGE_DEFAULT;
			texture2d_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			texture2d_desc.CPUAccessFlags     = 0;
			texture2d_desc.MiscFlags          = 0;
			ORB_CHECK_HRESULT( d3d11.device->CreateTexture2D( &texture2d_desc, nullptr, &texture2d.texture2d.ptr_ ) );

			// Create render target view
			D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
			render_target_view_desc.Format             = texture2d_desc.Format;
			render_target_view_desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
			render_target_view_desc.Texture2D.MipSlice = 0;
			ORB_CHECK_HRESULT( d3d11.device->CreateRenderTargetView( texture2d.texture2d.ptr_, &render_target_view_desc, &details.render_target_view.ptr_ ) );

			// Create shader resource view
			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
			shader_resource_view_desc.Format                    = texture2d_desc.Format;
			shader_resource_view_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_desc.Texture2D.MipLevels       = 1;
			shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
			ORB_CHECK_HRESULT( d3d11.device->CreateShaderResourceView( texture2d.texture2d.ptr_, &shader_resource_view_desc, &texture2d.shader_resource_view.ptr_ ) );

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_FrameBufferDetailsOpenGL, Private::FrameBufferDetails > ):
		{
			auto& details   = std::get< Private::_FrameBufferDetailsOpenGL >( framebuffer_details_ );
			auto& texture2d = std::get< Private::_Texture2DDetailsOpenGL >( texture2d_.GetPrivateDetails() );

			glBindFramebuffer( OpenGLFramebufferTarget::Draw, details.fbo );

			glBindTexture( GL_TEXTURE_2D, texture2d.id );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ORB_GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ORB_GL_CLAMP_TO_EDGE );
			glFramebufferTexture2D( OpenGLFramebufferTarget::Draw, OpenGLFramebufferAttachment::Color0, GL_TEXTURE_2D, texture2d.id, 0 );
			glBindTexture( GL_TEXTURE_2D, 0 );

			glBindRenderbuffer( OpenGLRenderbufferTarget::Renderbuffer, details.rbo );
			glRenderbufferStorage( OpenGLRenderbufferTarget::Renderbuffer, ORB_GL_DEPTH24_STENCIL8, width, height );
			glFramebufferRenderbuffer( OpenGLFramebufferTarget::Draw, OpenGLFramebufferAttachment::DepthStencil, OpenGLRenderbufferTarget::Renderbuffer, details.rbo );
			glBindRenderbuffer( OpenGLRenderbufferTarget::Renderbuffer, 0 );

			glBindFramebuffer( OpenGLFramebufferTarget::Draw, 0 );

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

ORB_NAMESPACE_END
