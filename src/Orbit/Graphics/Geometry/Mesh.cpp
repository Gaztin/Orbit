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

#include "Mesh.h"

#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Context/RenderContext.h"
#include "Orbit/Graphics/Geometry/Geometry.h"

ORB_NAMESPACE_BEGIN

Mesh::Mesh( std::string_view name )
	: name_( name )
{
}

Geometry Mesh::ToGeometry( void ) const
{
	Geometry geometry( vertex_layout_ );
	auto&    vb_details = vertex_buffer_->GetDetails();

	switch( vb_details.index() )
	{

#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_VertexBufferDetailsD3D11, Private::VertexBufferDetails > ):
		{
			auto&                    context  = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto&                    vb_d3d11 = std::get< Private::_VertexBufferDetailsD3D11 >( vb_details );
			D3D11_BUFFER_DESC        temp_vb_desc;
			D3D11_MAPPED_SUBRESOURCE temp_vb_mapped;
			ComPtr< ID3D11Buffer >   temp_vb;

			vb_d3d11.buffer->GetDesc( &temp_vb_desc );
			temp_vb_desc.Usage          = D3D11_USAGE_DEFAULT;
			temp_vb_desc.BindFlags      = D3D11_BIND_UNORDERED_ACCESS;
			temp_vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			context.device->CreateBuffer( &temp_vb_desc, nullptr, &temp_vb.ptr_ );
			context.device_context->CopyResource( temp_vb.ptr_, vb_d3d11.buffer.ptr_ );
			context.device_context->Map( temp_vb.ptr_, 0, D3D11_MAP_READ, 0, &temp_vb_mapped );

//////////////////////////////////////////////////////////////////////////

			if( index_buffer_ )
			{
				auto&                    ib_d3d11 = std::get< Private::_IndexBufferDetailsD3D11 >( index_buffer_->GetDetails() );
				D3D11_BUFFER_DESC        temp_ib_desc;
				D3D11_MAPPED_SUBRESOURCE temp_ib_mapped;
				ComPtr< ID3D11Buffer >   temp_ib;

				ib_d3d11.buffer->GetDesc( &temp_ib_desc );
				temp_ib_desc.Usage          = D3D11_USAGE_DEFAULT;
				temp_ib_desc.BindFlags      = D3D11_BIND_UNORDERED_ACCESS;
				temp_ib_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

				context.device->CreateBuffer( &temp_ib_desc, nullptr, &temp_ib.ptr_ );
				context.device_context->CopyResource( temp_ib.ptr_, ib_d3d11.buffer.ptr_ );
				context.device_context->Map( temp_ib.ptr_, 0, D3D11_MAP_READ, 0, &temp_ib_mapped );

				// Supply geometry with vertex and index data
				geometry.SetFromData( { static_cast< const uint8_t* >( temp_vb_mapped.pData ), vertex_buffer_->GetSize() }, { static_cast< const uint8_t* >( temp_ib_mapped.pData ), index_buffer_->GetSize() }, index_buffer_->GetFormat() );

				context.device_context->Unmap( temp_ib.ptr_, 0 );
			}
			else
			{
				// Supply geometry with just vertex data
				geometry.SetFromData( { static_cast< const uint8_t* >( temp_vb_mapped.pData ), temp_vb_desc.ByteWidth } );
			}

			context.device_context->Unmap( temp_vb.ptr_, 0 );

		} break;

#endif // ORB_HAS_D3D11
#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto&        vb_opengl = std::get< Private::_VertexBufferDetailsOpenGL >( vb_details );
			const size_t vb_size   = ( vertex_buffer_->GetCount() * vertex_layout_.GetStride() );

			glBindBuffer( OpenGLBufferTarget::Array, vb_opengl.id );
			const void*  vb_src    = glMapBufferRange( OpenGLBufferTarget::Array, 0, vb_size, OpenGLMapAccess::ReadBit );

			if( index_buffer_ )
			{
				auto&        ib_opengl = std::get< Private::_IndexBufferDetailsOpenGL >( index_buffer_->GetDetails() );
				const size_t ib_size   = index_buffer_->GetSize();

				glBindBuffer( OpenGLBufferTarget::ElementArray, ib_opengl.id );
				const void*  ib_src    = glMapBufferRange( OpenGLBufferTarget::ElementArray, 0, ib_size, OpenGLMapAccess::ReadBit );

				// Supply geometry with vertex and index data
				geometry.SetFromData( { static_cast< const uint8_t* >( vb_src ), vb_size }, { static_cast< const uint8_t* >( ib_src ), ib_size }, index_buffer_->GetFormat() );

				glUnmapBuffer( OpenGLBufferTarget::ElementArray );
				glBindBuffer( OpenGLBufferTarget::ElementArray, 0 );
			}
			else
			{
				// Supply geometry with just vertex data
				geometry.SetFromData( { static_cast< const uint8_t* >( vb_src ), vb_size } );
			}

			glUnmapBuffer( OpenGLBufferTarget::Array );
			glBindBuffer( OpenGLBufferTarget::Array, 0 );

		} break;

#endif // ORB_HAS_OPENGL

	}

	return geometry;
}

ORB_NAMESPACE_END
