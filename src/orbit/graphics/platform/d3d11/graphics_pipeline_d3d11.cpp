/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "graphics_pipeline_d3d11.h"

#include <utility>

#include "orbit/graphics/platform/d3d11/shader_pixel_d3d11.h"
#include "orbit/graphics/platform/d3d11/shader_vertex_d3d11.h"
#include "orbit/graphics/index_buffer.h"
#include "orbit/graphics/shader.h"
#include "orbit/graphics/vertex_buffer.h"

namespace orb
{
	namespace platform
	{
		static DXGI_FORMAT get_vertex_component_format( vertex_component::type_t type )
		{
			switch( type )
			{
				case vertex_component::Float: return DXGI_FORMAT_R32_FLOAT;
				case vertex_component::Vec2:  return DXGI_FORMAT_R32G32_FLOAT;
				case vertex_component::Vec3:  return DXGI_FORMAT_R32G32B32_FLOAT;
				case vertex_component::Vec4:  return DXGI_FORMAT_R32G32B32A32_FLOAT;
				default:                      return DXGI_FORMAT_UNKNOWN;
			}
		}

		graphics_pipeline_d3d11::graphics_pipeline_d3d11()
			: m_vertexData   ( nullptr )
			, m_vertexShader ( nullptr )
			, m_pixelShader  ( nullptr )
		{
		}

		void graphics_pipeline_d3d11::bind()
		{
			ID3D11DeviceContext& dc = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device_context();
			if( m_inputLayout )
				dc.IASetInputLayout( m_inputLayout.get() );
			if( m_vertexShader )
				dc.VSSetShader( m_vertexShader, nullptr, 0 );
			if( m_pixelShader )
				dc.PSSetShader( m_pixelShader, nullptr, 0 );
		}

		void graphics_pipeline_d3d11::unbind()
		{
			ID3D11DeviceContext& dc = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device_context();
			dc.PSSetShader( nullptr, nullptr, 0 );
			dc.VSSetShader( nullptr, nullptr, 0 );
			dc.IASetInputLayout( nullptr );
		}

		void graphics_pipeline_d3d11::add_shader( const shader& shr )
		{
			switch( shr.get_type() )
			{
				case shader_type::Vertex:
				{
					const shader_vertex_d3d11& shr_d3d11 = reinterpret_cast< const shader_vertex_d3d11& >( shr.get_base() );
					m_vertexData   = const_cast< ID3DBlob* >( &shr_d3d11.get_data() );
					m_vertexShader = const_cast< ID3D11VertexShader* >( &shr_d3d11.get_vertex_shader() );
					break;
				}

				case shader_type::Fragment:
				{
					const shader_pixel_d3d11& shr_d3d11 = reinterpret_cast< const shader_pixel_d3d11& >( shr.get_base() );
					m_pixelShader = const_cast< ID3D11PixelShader* >( &shr_d3d11.get_pixel_shader() );
					break;
				}
			}
		}

		void graphics_pipeline_d3d11::describe_vertex_layout( vertex_layout layout )
		{
			if( !m_vertexData )
			{
				log_error( "Failed to describe vertex layout in graphics pipeline. Vertex shader missing." );
				return;
			}

			std::vector< D3D11_INPUT_ELEMENT_DESC > descriptors;
			descriptors.reserve( layout.size() );
			for( const vertex_component& cmp : layout )
			{
				D3D11_INPUT_ELEMENT_DESC desc = { };
				desc.SemanticName      = cmp.semanticName.c_str();
				desc.AlignedByteOffset = descriptors.empty() ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
				desc.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
				desc.Format            = get_vertex_component_format( cmp.type );

				descriptors.emplace_back( std::move( desc ) );
			}

			ID3D11Device&      device      = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device();
			ID3D11InputLayout* inputLayout = nullptr;
			device.CreateInputLayout( descriptors.data(), static_cast< UINT >( descriptors.size() ), m_vertexData->GetBufferPointer(), m_vertexData->GetBufferSize(), &inputLayout );
			m_inputLayout.reset( inputLayout );
		}

		void graphics_pipeline_d3d11::draw( const vertex_buffer& vb )
		{
			ID3D11DeviceContext& dc = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device_context();
			dc.Draw( static_cast< UINT >( vb.get_count() ), 0 );
		}

		void graphics_pipeline_d3d11::draw( const index_buffer& ib )
		{
			ID3D11DeviceContext& dc = static_cast< render_context_d3d11& >( render_context::get_current()->get_base() ).get_device_context();
			dc.DrawIndexed( static_cast< UINT >( ib.get_count() ), 0, 0 );
		}
	}
}
