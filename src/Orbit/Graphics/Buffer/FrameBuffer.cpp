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
#include "Orbit/Graphics/Context/RenderContext.h"

ORB_NAMESPACE_BEGIN

FrameBuffer::FrameBuffer( void )
{
	auto& context = RenderContext::GetInstance().GetPrivateDetails();

	memset( &opaque_texture_2d_, 0, sizeof( Texture2D ) );

	on_resize_ = Window::GetInstance().Subscribe( [ this ]( const WindowResized& e ){ Resize( e.width, e.height ); } );

	switch( context.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& texture2d = GetTexture2D().GetPrivateDetails();

			details_.emplace< Private::_FrameBufferDetailsD3D11 >();
			texture2d.emplace< Private::_Texture2DDetailsD3D11 >();

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

void FrameBuffer::Clear( void )
{
	switch( details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_FrameBufferDetailsD3D11, Private::FrameBufferDetails > ):
		{
			auto&       d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto&       details = std::get< Private::_FrameBufferDetailsD3D11 >( details_ );
			const FLOAT clear_color[ 4 ]{ };

			d3d11.device_context->ClearRenderTargetView( details.render_target_view.ptr_, clear_color );

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_FrameBufferDetailsOpenGL, Private::FrameBufferDetails > ):
		{

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

void FrameBuffer::Bind( void )
{
	switch( details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_FrameBufferDetailsD3D11, Private::FrameBufferDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details = std::get< Private::_FrameBufferDetailsD3D11 >( details_ );

			ORB_CHECK_SYSTEM_ERROR( d3d11.device_context->OMSetRenderTargets( 1, &details.render_target_view.ptr_, d3d11.depth_stencil_view.ptr_ ) );

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_FrameBufferDetailsOpenGL, Private::FrameBufferDetails > ):
		{

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

void FrameBuffer::Unbind( void )
{
	switch( details_.index() )
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

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

Texture2D& FrameBuffer::GetTexture2D( void )
{
	return *reinterpret_cast< Texture2D* >( &opaque_texture_2d_ );
}

void FrameBuffer::Resize( uint32_t width, uint32_t height )
{
	switch( details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_FrameBufferDetailsD3D11, Private::FrameBufferDetails > ):
		{
			auto& d3d11     = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details   = std::get< Private::_FrameBufferDetailsD3D11 >( details_ );
			auto& texture2d = std::get< Private::_Texture2DDetailsD3D11 >( GetTexture2D().GetPrivateDetails() );

			details.buffer               = nullptr;
			details.render_target_view   = nullptr;
			details.shader_resource_view = nullptr;

			D3D11_TEXTURE2D_DESC buffer_desc;
			buffer_desc.Width              = width;
			buffer_desc.Height             = height;
			buffer_desc.MipLevels          = 0;
			buffer_desc.ArraySize          = 1;
			buffer_desc.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
			buffer_desc.SampleDesc.Count   = 1;
			buffer_desc.SampleDesc.Quality = 0;
			buffer_desc.Usage              = D3D11_USAGE_DEFAULT;
			buffer_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			buffer_desc.CPUAccessFlags     = 0;
			buffer_desc.MiscFlags          = 0;
			ORB_CHECK_HRESULT( d3d11.device->CreateTexture2D( &buffer_desc, nullptr, &details.buffer.ptr_ ) );

			D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
			render_target_view_desc.Format             = buffer_desc.Format;
			render_target_view_desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
			render_target_view_desc.Texture2D.MipSlice = 0;
			ORB_CHECK_HRESULT( d3d11.device->CreateRenderTargetView( details.buffer.ptr_, &render_target_view_desc, &details.render_target_view.ptr_ ) );

			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
			shader_resource_view_desc.Format                    = buffer_desc.Format;
			shader_resource_view_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_desc.Texture2D.MipLevels       = 1;
			shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
			ORB_CHECK_HRESULT( d3d11.device->CreateShaderResourceView( details.buffer.ptr_, &shader_resource_view_desc, &details.shader_resource_view.ptr_ ) );

			texture2d.texture_2d           = details.buffer;
			texture2d.shader_resource_view = details.shader_resource_view;

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_FrameBufferDetailsOpenGL, Private::FrameBufferDetails > ):
		{

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

ORB_NAMESPACE_END