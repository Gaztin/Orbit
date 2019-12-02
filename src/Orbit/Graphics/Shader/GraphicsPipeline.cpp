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

#include "GraphicsPipeline.h"

#include "Orbit/Core/IO/Log.h"
#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Device/RenderContext.h"
#include "Orbit/Graphics/Shader/FragmentShader.h"
#include "Orbit/Graphics/Shader/VertexShader.h"

ORB_NAMESPACE_BEGIN

GraphicsPipeline::GraphicsPipeline( void )
	: m_impl { }
{
	auto& context_impl_var = RenderContext::GetCurrent()->GetPrivateImpl();

	switch( context_impl_var.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextImplOpenGL, Private::RenderContextImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( context_impl_var );

			if( ( context_impl.embedded && context_impl.opengl_version < Version( 3 ) ) || ( !context_impl.embedded && context_impl.opengl_version < Version( 3, 0 ) ) )
			{
				auto impl = std::addressof( m_impl.emplace< Private::_GraphicsPipelineImplOpenGL20 >() );

				impl->stride         = 0;
				impl->shader_program = context_impl.functions->create_program();
			}
			else
			{
				auto impl = std::addressof( m_impl.emplace< Private::_GraphicsPipelineImplOpenGL30 >() );

				impl->stride         = 0;
				impl->shader_program = context_impl.functions->create_program();
				context_impl.functions->gen_vertex_arrays( 1, &impl->vao );
			}

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextImplD3D11, Private::RenderContextImpl > ):
		{
			auto& impl         = m_impl.emplace< Private::_GraphicsPipelineImplD3D11 >();
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			D3D11_SAMPLER_DESC desc { };
			desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.MipLODBias       = 0.0f;
			desc.MaxAnisotropy    = 1;
			desc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
			desc.BorderColor[ 0 ] = 0;
			desc.BorderColor[ 1 ] = 0;
			desc.BorderColor[ 2 ] = 0;
			desc.BorderColor[ 3 ] = 0;
			desc.MinLOD           = 0;
			desc.MaxLOD           = D3D11_FLOAT32_MAX;

			ID3D11SamplerState* sampler_state;
			if( context_impl.device->CreateSamplerState( &desc, &sampler_state ) == S_OK )
			{
				impl.sampler_state.reset( sampler_state );
			}

			break;
		}

	#endif

	}
}

GraphicsPipeline::~GraphicsPipeline( void )
{
	switch( m_impl.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL20, Private::GraphicsPipelineImpl > ):
		{
			auto& impl         = std::get< Private::_GraphicsPipelineImplOpenGL20 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->delete_program( impl.shader_program );

			break;
		}

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL30, Private::GraphicsPipelineImpl > ):
		{
			auto& impl         = std::get< Private::_GraphicsPipelineImplOpenGL30 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->delete_program( impl.shader_program );
			context_impl.functions->delete_vertex_arrays( 1, &impl.vao );

			break;
		}

	#endif

	}
}

void GraphicsPipeline::Bind( void )
{
	switch( m_impl.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL20, Private::GraphicsPipelineImpl > ):
		{
			auto& impl         = std::get< Private::_GraphicsPipelineImplOpenGL20 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->use_program( impl.shader_program );

			const uint8_t* ptr = nullptr;
			for( GLuint i = 0; i < impl.layout.size(); ++i )
			{
				OpenGL::VertexAttribDataType data_type { };
				GLint                        data_length = 0;
				VertexComponent::Type        type        = impl.layout[ i ].type;

				switch( type )
				{
					default:
					{
						LogError( "Invalid attrib data type" );
						continue;
					}

					case VertexComponent::Float:
					{
						data_type   = OpenGL::VertexAttribDataType::Float;
						data_length = 1;
						break;
					}

					case VertexComponent::Vec2:
					{
						data_type   = OpenGL::VertexAttribDataType::Float;
						data_length = 2;
						break;
					}

					case VertexComponent::Vec3:
					{
						data_type   = OpenGL::VertexAttribDataType::Float;
						data_length = 3;
						break;
					}

					case VertexComponent::Vec4:
					{
						data_type   = OpenGL::VertexAttribDataType::Float;
						data_length = 4;
						break;
					}
				}

				context_impl.functions->enable_vertex_attrib_array( i );
				context_impl.functions->vertex_attrib_pointer( i, data_length, data_type, GL_FALSE, impl.stride, ptr );

				switch( type )
				{
					case VertexComponent::Float: { ptr += sizeof( float ) * 1; } break;
					case VertexComponent::Vec2:  { ptr += sizeof( float ) * 2; } break;
					case VertexComponent::Vec3:  { ptr += sizeof( float ) * 3; } break;
					case VertexComponent::Vec4:  { ptr += sizeof( float ) * 4; } break;
				}
			}

			break;
		}
		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL30, Private::GraphicsPipelineImpl > ):
		{
			auto& impl         = std::get< Private::_GraphicsPipelineImplOpenGL30 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->bind_vertex_array( impl.vao );
			context_impl.functions->use_program( impl.shader_program );

			const uint8_t* ptr = nullptr;
			for( GLuint i = 0; i < impl.layout.size(); ++i )
			{
				OpenGL::VertexAttribDataType data_type { };
				GLint                        data_length = 0;
				VertexComponent::Type        type        = impl.layout[ i ].type;

				switch( type )
				{
					default:
					{
						LogError( "Invalid attrib data type" );
						continue;
					}

					case VertexComponent::Float:
					{
						data_type   = OpenGL::VertexAttribDataType::Float;
						data_length = 1;
						break;
					}

					case VertexComponent::Vec2:
					{
						data_type   = OpenGL::VertexAttribDataType::Float;
						data_length = 2;
						break;
					}

					case VertexComponent::Vec3:
					{
						data_type   = OpenGL::VertexAttribDataType::Float;
						data_length = 3;
						break;
					}

					case VertexComponent::Vec4:
					{
						data_type   = OpenGL::VertexAttribDataType::Float;
						data_length = 4;
						break;
					}
					}

				context_impl.functions->enable_vertex_attrib_array( i );
				context_impl.functions->vertex_attrib_pointer( i, data_length, data_type, GL_FALSE, impl.stride, ptr );

				switch( type )
				{
					case VertexComponent::Float: { ptr += sizeof( float ) * 1; } break;
					case VertexComponent::Vec2:  { ptr += sizeof( float ) * 2; } break;
					case VertexComponent::Vec3:  { ptr += sizeof( float ) * 3; } break;
					case VertexComponent::Vec4:  { ptr += sizeof( float ) * 4; } break;
				}
			}

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_GraphicsPipelineImplD3D11, Private::GraphicsPipelineImpl > ):
		{
			auto& impl         = std::get< Private::_GraphicsPipelineImplD3D11 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			if( impl.input_layout )  context_impl.device_context->IASetInputLayout( impl.input_layout.get() );
			if( impl.vertex_shader ) context_impl.device_context->VSSetShader( impl.vertex_shader.get(), nullptr, 0 );
			if( impl.pixel_shader )  context_impl.device_context->PSSetShader( impl.pixel_shader.get(), nullptr, 0 );

			if( impl.sampler_state )
			{
				ID3D11SamplerState* sampler_state = impl.sampler_state.get();
				context_impl.device_context->PSSetSamplers( 0, 1, &sampler_state );
			}

			break;
		}

	#endif

	}
}

void GraphicsPipeline::Unbind( void )
{
	switch( m_impl.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL20, Private::GraphicsPipelineImpl > ):
		{
			auto& impl         = std::get< Private::_GraphicsPipelineImplOpenGL20 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			for( GLuint i = 0; i < impl.layout.size(); ++i )
				context_impl.functions->disable_vertex_attrib_array( i );

			context_impl.functions->use_program( 0 );

			break;
		}

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL30, Private::GraphicsPipelineImpl > ):
		{
			auto& impl         = std::get< Private::_GraphicsPipelineImplOpenGL30 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			for( GLuint i = 0; i < impl.layout.size(); ++i )
				context_impl.functions->disable_vertex_attrib_array( i );

			context_impl.functions->use_program( 0 );
			context_impl.functions->bind_vertex_array( 0 );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_GraphicsPipelineImplD3D11, Private::GraphicsPipelineImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.device_context->PSSetSamplers( 0, 0, nullptr );
			context_impl.device_context->IASetInputLayout( nullptr );
			context_impl.device_context->VSSetShader( nullptr, nullptr, 0 );
			context_impl.device_context->PSSetShader( nullptr, nullptr, 0 );

			break;
		}

	#endif

	}
}

void GraphicsPipeline::SetShaders( const VertexShader& vert, const FragmentShader& frag )
{
	switch( m_impl.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL20, Private::GraphicsPipelineImpl > ):
		{
			auto& impl          = std::get< Private::_GraphicsPipelineImplOpenGL20 >( m_impl );
			auto& impl_vert     = std::get< Private::_VertexShaderImplOpenGL >( vert.GetPrivateImpl() );
			auto& impl_frag     = std::get< Private::_FragmentShaderImplOpenGL >( frag.GetPrivateImpl() );
			auto& constext_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			constext_impl.functions->attach_shader( impl.shader_program, impl_vert.id );
			constext_impl.functions->attach_shader( impl.shader_program, impl_frag.id );
			constext_impl.functions->link_program( impl.shader_program );

			GLint loglen = 0;
			constext_impl.functions->get_programiv( impl.shader_program, OpenGL::ProgramParam::InfoLogLength, &loglen );
			if( loglen > 0 )
			{
				std::string logbuf( static_cast< size_t >( loglen ), '\0' );
				constext_impl.functions->get_program_info_log( impl.shader_program, loglen, nullptr, &logbuf[ 0 ] );
				( logbuf );
			}

			break;
		}

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL30, Private::GraphicsPipelineImpl > ):
		{
			auto& impl          = std::get< Private::_GraphicsPipelineImplOpenGL30 >( m_impl );
			auto& impl_vert     = std::get< Private::_VertexShaderImplOpenGL >( vert.GetPrivateImpl() );
			auto& impl_frag     = std::get< Private::_FragmentShaderImplOpenGL >( frag.GetPrivateImpl() );
			auto& constext_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			constext_impl.functions->attach_shader( impl.shader_program, impl_vert.id );
			constext_impl.functions->attach_shader( impl.shader_program, impl_frag.id );
			constext_impl.functions->link_program( impl.shader_program );

			GLint loglen = 0;
			constext_impl.functions->get_programiv( impl.shader_program, OpenGL::ProgramParam::InfoLogLength, &loglen );
			if( loglen > 0 )
			{
				std::string logbuf( static_cast< size_t >( loglen ), '\0' );
				constext_impl.functions->get_program_info_log( impl.shader_program, loglen, nullptr, &logbuf[ 0 ] );
				LogError( logbuf );
			}

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_GraphicsPipelineImplD3D11, Private::GraphicsPipelineImpl > ):
		{
			auto& impl      = std::get< Private::_GraphicsPipelineImplD3D11 >( m_impl );
			auto& impl_vert = std::get< Private::_VertexShaderImplD3D11 >( vert.GetPrivateImpl() );
			auto& impl_frag = std::get< Private::_FragmentShaderImplD3D11 >( frag.GetPrivateImpl() );

			impl.vertex_data.reset( impl_vert.vertex_data.get() );
			impl.vertex_data->AddRef();

			impl.vertex_shader.reset( impl_vert.vertex_shader.get() );
			impl.vertex_shader->AddRef();

			impl.pixel_shader.reset( impl_frag.pixel_shader.get() );
			impl.pixel_shader->AddRef();

			break;
		}

	#endif

	}
}

void GraphicsPipeline::DescribeVertexLayout( const VertexLayout& layout )
{
	switch( m_impl.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL20, Private::GraphicsPipelineImpl > ):
		{
			auto& impl = std::get< Private::_GraphicsPipelineImplOpenGL20 >( m_impl );

			impl.layout = layout;

			/* Calculate stride */
			impl.stride = 0;
			for( const VertexComponent& cmp : impl.layout )
			{
				switch( cmp.type )
				{
					case VertexComponent::Float: { impl.stride += sizeof( float ) * 1; } break;
					case VertexComponent::Vec2:  { impl.stride += sizeof( float ) * 2; } break;
					case VertexComponent::Vec3:  { impl.stride += sizeof( float ) * 3; } break;
					case VertexComponent::Vec4:  { impl.stride += sizeof( float ) * 4; } break;
				}
			}

			break;
		}

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL30, Private::GraphicsPipelineImpl > ):
		{
			auto& impl = std::get< Private::_GraphicsPipelineImplOpenGL30 >( m_impl );

			impl.layout = layout;

			/* Calculate stride */
			impl.stride = 0;
			for( const VertexComponent& cmp : impl.layout )
			{
				switch( cmp.type )
				{
					case VertexComponent::Float: { impl.stride += sizeof( float ) * 1; } break;
					case VertexComponent::Vec2:  { impl.stride += sizeof( float ) * 2; } break;
					case VertexComponent::Vec3:  { impl.stride += sizeof( float ) * 3; } break;
					case VertexComponent::Vec4:  { impl.stride += sizeof( float ) * 4; } break;
				}
			}

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_GraphicsPipelineImplD3D11, Private::GraphicsPipelineImpl > ):
		{
			auto& impl         = std::get< Private::_GraphicsPipelineImplD3D11 >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			if( !impl.vertex_data )
			{
				LogError( "Failed to describe vertex layout in graphics pipeline. Vertex shader missing." );
				break;
			}

			std::vector< D3D11_INPUT_ELEMENT_DESC > descriptors;
			descriptors.reserve( layout.size() );

			for( const VertexComponent& cmp : layout )
			{
				D3D11_INPUT_ELEMENT_DESC desc{ };
				desc.SemanticName      = cmp.semantic_name.c_str();
				desc.AlignedByteOffset = descriptors.empty() ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
				desc.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

				switch( cmp.type )
				{
					default:                     { desc.Format = DXGI_FORMAT_UNKNOWN;            } break;
					case VertexComponent::Float: { desc.Format = DXGI_FORMAT_R32_FLOAT;          } break;
					case VertexComponent::Vec2:  { desc.Format = DXGI_FORMAT_R32G32_FLOAT;       } break;
					case VertexComponent::Vec3:  { desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;    } break;
					case VertexComponent::Vec4:  { desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; } break;
				}

				descriptors.push_back( desc );
			}

			ID3D11InputLayout* input_layout = nullptr;
			context_impl.device->CreateInputLayout( descriptors.data(), static_cast< UINT >( descriptors.size() ), impl.vertex_data->GetBufferPointer(), impl.vertex_data->GetBufferSize(), &input_layout );
			impl.input_layout.reset( input_layout );

			break;
		}

	#endif

	}
}

void GraphicsPipeline::Draw( const VertexBuffer& vb )
{
	switch( m_impl.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL20, Private::GraphicsPipelineImpl > ):
		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL30, Private::GraphicsPipelineImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );
			context_impl.functions->draw_arrays( OpenGL::DrawMode::Triangles, 0, static_cast< GLsizei >( vb.GetCount() ) );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_GraphicsPipelineImplD3D11, Private::GraphicsPipelineImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.device_context->Draw( static_cast< UINT >( vb.GetCount() ), 0 );

			break;
		}

	#endif

	}
}

void GraphicsPipeline::Draw( const IndexBuffer& ib )
{
	switch( m_impl.index() )
	{

	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL20, Private::GraphicsPipelineImpl > ):
		case( unique_index_v< Private::_GraphicsPipelineImplOpenGL30, Private::GraphicsPipelineImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			OpenGL::IndexType index_type { };
			switch( ib.GetFormat() )
			{
				case IndexFormat::Byte:       { index_type = OpenGL::IndexType::Byte;  } break;
				case IndexFormat::Word:       { index_type = OpenGL::IndexType::Short; } break;
				case IndexFormat::DoubleWord: { index_type = OpenGL::IndexType::Int;   } break;
			}

			context_impl.functions->draw_elements( OpenGL::DrawMode::Triangles, static_cast< GLsizei >( ib.GetCount() ), index_type, nullptr );

			break;
		}

	#endif
	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_GraphicsPipelineImplD3D11, Private::GraphicsPipelineImpl > ):
		{
			auto& context_impl = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.device_context->DrawIndexed( static_cast< UINT >( ib.GetCount() ), 0, 0 );

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
