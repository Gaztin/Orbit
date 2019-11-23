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

#include "graphics_pipeline.h"

#include "orbit/core/log.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/fragment_shader.h"
#include "orbit/graphics/index_buffer.h"
#include "orbit/graphics/render_context.h"
#include "orbit/graphics/vertex_buffer.h"
#include "orbit/graphics/vertex_shader.h"

namespace orb
{
	template< typename T >
	constexpr auto render_context_impl_index_v = unique_index_v< T, render_context_impl >;

	template< typename T >
	constexpr auto graphics_pipeline_impl_index_v = unique_index_v< T, graphics_pipeline_impl >;

	graphics_pipeline::graphics_pipeline()
		: m_impl{ }
	{
		auto currentContextImpl = render_context::get_current()->get_impl_ptr();
		switch( currentContextImpl->index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				auto  implCtx   = std::get_if< __render_context_impl_opengl >( currentContextImpl );
				auto& functions = implCtx->functions.value();

				if( ( implCtx->embedded && implCtx->glVersion < version( 3 ) ) || ( !implCtx->embedded && implCtx->glVersion < version( 3, 0 ) ) )
				{
					auto impl           = std::addressof( m_impl.emplace< __graphics_pipeline_impl_opengl_2_0 >() );
					impl->stride        = 0;
					impl->shaderProgram = functions.create_program();
				}
				else
				{
					auto impl           = std::addressof( m_impl.emplace< __graphics_pipeline_impl_opengl_3_0 >() );
					impl->stride        = 0;
					impl->shaderProgram = functions.create_program();
					functions.gen_vertex_arrays( 1, &impl->vao );
				}

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
			{
				m_impl.emplace< __graphics_pipeline_impl_d3d11 >();
				break;
			}
		#endif
		}
	}

	graphics_pipeline::~graphics_pipeline()
	{
		switch( m_impl.index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_2_0 > ):
			{
				auto  impl      = std::get_if< __graphics_pipeline_impl_opengl_2_0 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.delete_program( impl->shaderProgram );

				break;
			}
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_3_0 > ):
			{
				auto  impl      = std::get_if< __graphics_pipeline_impl_opengl_3_0 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.delete_program( impl->shaderProgram );
				functions.delete_vertex_arrays( 1, &impl->vao );

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
//			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_d3d11 > ):
//			{
//				break;
//			}
		#endif
		}
	}

	void graphics_pipeline::bind()
	{
		switch( m_impl.index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_2_0 > ):
			{
				auto  impl      = std::get_if< __graphics_pipeline_impl_opengl_2_0 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.use_program( impl->shaderProgram );

				const uint8_t* ptr = nullptr;
				for( GLuint i = 0; i < impl->layout.size(); ++i )
				{
					gl::vertex_attrib_data_type dataType{ };
					GLint                       dataLength = 0;
					switch( impl->layout.at( i ).type )
					{
						default:
						case vertex_component::Float:
							dataType   = gl::vertex_attrib_data_type::Float;
							dataLength = 1;
							break;

						case vertex_component::Vec2:
							dataType   = gl::vertex_attrib_data_type::Float;
							dataLength = 2;
							break;

						case vertex_component::Vec3:
							dataType   = gl::vertex_attrib_data_type::Float;
							dataLength = 3;
							break;

						case vertex_component::Vec4:
							dataType   = gl::vertex_attrib_data_type::Float;
							dataLength = 4;
							break;
					}

					functions.enable_vertex_attrib_array( i );
					functions.vertex_attrib_pointer( i, dataLength, dataType, GL_FALSE, impl->stride, ptr );

					switch( impl->layout.at( i ).type )
					{
						default:
						case vertex_component::Float: ptr += sizeof( float ) * 1; break;
						case vertex_component::Vec2:  ptr += sizeof( float ) * 2; break;
						case vertex_component::Vec3:  ptr += sizeof( float ) * 3; break;
						case vertex_component::Vec4:  ptr += sizeof( float ) * 4; break;
					}
				}

				break;
			}
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_3_0 > ):
			{
				auto  impl      = std::get_if< __graphics_pipeline_impl_opengl_3_0 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.bind_vertex_array( impl->vao );
				functions.use_program( impl->shaderProgram );

				const uint8_t* ptr = nullptr;
				for( GLuint i = 0; i < impl->layout.size(); ++i )
				{
					gl::vertex_attrib_data_type dataType{ };
					GLint                       dataLength = 0;
					switch( impl->layout.at( i ).type )
					{
						default:
						case vertex_component::Float:
							dataType   = gl::vertex_attrib_data_type::Float;
							dataLength = 1;
							break;

						case vertex_component::Vec2:
							dataType   = gl::vertex_attrib_data_type::Float;
							dataLength = 2;
							break;

						case vertex_component::Vec3:
							dataType   = gl::vertex_attrib_data_type::Float;
							dataLength = 3;
							break;

						case vertex_component::Vec4:
							dataType   = gl::vertex_attrib_data_type::Float;
							dataLength = 4;
							break;
					}

					functions.enable_vertex_attrib_array( i );
					functions.vertex_attrib_pointer( i, dataLength, dataType, GL_FALSE, impl->stride, ptr );

					switch( impl->layout.at( i ).type )
					{
						default:
						case vertex_component::Float: ptr += sizeof( float ) * 1; break;
						case vertex_component::Vec2:  ptr += sizeof( float ) * 2; break;
						case vertex_component::Vec3:  ptr += sizeof( float ) * 3; break;
						case vertex_component::Vec4:  ptr += sizeof( float ) * 4; break;
					}
				}

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_d3d11 > ):
			{
				auto                 impl          = std::get_if< __graphics_pipeline_impl_d3d11 >( &m_impl );
				ID3D11DeviceContext& deviceContext = *( std::get_if< __render_context_impl_d3d11 >( render_context::get_current()->get_impl_ptr() )->deviceContext );

				if( impl->inputLayout )
					deviceContext.IASetInputLayout( impl->inputLayout.get() );
				if( impl->vertexShader )
					deviceContext.VSSetShader( impl->vertexShader.get(), nullptr, 0 );
				if( impl->pixelShader )
					deviceContext.PSSetShader( impl->pixelShader.get(), nullptr, 0 );

				break;
			}
		#endif
		}
	}

	void graphics_pipeline::unbind()
	{
		switch( m_impl.index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_2_0 > ):
			{
				auto  impl      = std::get_if< __graphics_pipeline_impl_opengl_2_0 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				for( GLuint i = 0; i < impl->layout.size(); ++i )
					functions.disable_vertex_attrib_array( i );

				functions.use_program( 0 );

				break;
			}
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_3_0 > ):
			{
				auto  impl      = std::get_if< __graphics_pipeline_impl_opengl_3_0 >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				for( GLuint i = 0; i < impl->layout.size(); ++i )
					functions.disable_vertex_attrib_array( i );

				functions.use_program( 0 );
				functions.bind_vertex_array( 0 );

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_d3d11 > ):
			{
				ID3D11DeviceContext& deviceContext = *( std::get_if< __render_context_impl_d3d11 >( render_context::get_current()->get_impl_ptr() )->deviceContext );
				deviceContext.IASetInputLayout( nullptr );
				deviceContext.VSSetShader( nullptr, nullptr, 0 );
				deviceContext.PSSetShader( nullptr, nullptr, 0 );
				break;
			}
		#endif
		}
	}

	void graphics_pipeline::set_shaders( const vertex_shader& vert, const fragment_shader& frag )
	{
		switch( m_impl.index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_2_0 > ):
			{
				auto  impl      = std::get_if< __graphics_pipeline_impl_opengl_2_0 >( &m_impl );
				auto  implVert  = std::get_if< __vertex_shader_impl_opengl >( vert.get_impl_ptr() );
				auto  implFrag  = std::get_if< __fragment_shader_impl_opengl >( frag.get_impl_ptr() );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.attach_shader( impl->shaderProgram, implVert->id );
				functions.attach_shader( impl->shaderProgram, implFrag->id );
				functions.link_program( impl->shaderProgram );

				GLint loglen = 0;
				functions.get_programiv( impl->shaderProgram, gl::program_param::InfoLogLength, &loglen );
				if( loglen > 0 )
				{
					std::string logbuf( static_cast< size_t >( loglen ), '\0' );
					functions.get_program_info_log( impl->shaderProgram, loglen, nullptr, &logbuf[ 0 ] );
					log_error( logbuf );
				}

				break;
			}
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_3_0 > ):
			{
				auto  impl      = std::get_if< __graphics_pipeline_impl_opengl_3_0 >( &m_impl );
				auto  implVert  = std::get_if< __vertex_shader_impl_opengl >( vert.get_impl_ptr() );
				auto  implFrag  = std::get_if< __fragment_shader_impl_opengl >( frag.get_impl_ptr() );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.attach_shader( impl->shaderProgram, implVert->id );
				functions.attach_shader( impl->shaderProgram, implFrag->id );
				functions.link_program( impl->shaderProgram );

				GLint loglen = 0;
				functions.get_programiv( impl->shaderProgram, gl::program_param::InfoLogLength, &loglen );
				if( loglen > 0 )
				{
					std::string logbuf( static_cast< size_t >( loglen ), '\0' );
					functions.get_program_info_log( impl->shaderProgram, loglen, nullptr, &logbuf[ 0 ] );
					log_error( logbuf );
				}

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_d3d11 > ):
			{
				auto impl     = std::get_if< __graphics_pipeline_impl_d3d11 >( &m_impl );
				auto implVert = std::get_if< __vertex_shader_impl_d3d11 >( vert.get_impl_ptr() );
				auto implFrag = std::get_if< __fragment_shader_impl_d3d11 >( frag.get_impl_ptr() );

				impl->vertexData.reset( implVert->vertexData.get() );
				impl->vertexData->AddRef();

				impl->vertexShader.reset( implVert->vertexShader.get() );
				impl->vertexShader->AddRef();

				impl->pixelShader.reset( implFrag->pixelShader.get() );
				impl->pixelShader->AddRef();

				break;
			}
		#endif
		}
	}

	void graphics_pipeline::describe_vertex_layout( vertex_layout layout )
	{
		switch( m_impl.index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_2_0 > ):
			{
				auto impl = std::get_if< __graphics_pipeline_impl_opengl_2_0 >( &m_impl );

				impl->layout.assign( layout );

				/* Calculate stride */
				impl->stride = 0;
				for( const vertex_component& cmp : impl->layout )
				{
					switch( cmp.type )
					{
						default:
						case vertex_component::Float: impl->stride += sizeof( float ) * 1; break;
						case vertex_component::Vec2:  impl->stride += sizeof( float ) * 2; break;
						case vertex_component::Vec3:  impl->stride += sizeof( float ) * 3; break;
						case vertex_component::Vec4:  impl->stride += sizeof( float ) * 4; break;
					}
				}

				break;
			}
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_3_0 > ):
			{
				auto impl = std::get_if< __graphics_pipeline_impl_opengl_3_0 >( &m_impl );

				impl->layout.assign( layout );

				/* Calculate stride */
				impl->stride = 0;
				for( const vertex_component& cmp : impl->layout )
				{
					switch( cmp.type )
					{
						default:
						case vertex_component::Float: impl->stride += sizeof( float ) * 1; break;
						case vertex_component::Vec2:  impl->stride += sizeof( float ) * 2; break;
						case vertex_component::Vec3:  impl->stride += sizeof( float ) * 3; break;
						case vertex_component::Vec4:  impl->stride += sizeof( float ) * 4; break;
					}
				}

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_d3d11 > ):
			{
				auto impl = std::get_if< __graphics_pipeline_impl_d3d11 >( &m_impl );

				if( !impl->vertexData )
				{
					log_error( "Failed to describe vertex layout in graphics pipeline. Vertex shader missing." );
					break;
				}

				std::vector< D3D11_INPUT_ELEMENT_DESC > descriptors;
				descriptors.reserve( layout.size() );
				for( const vertex_component& cmp : layout )
				{
					D3D11_INPUT_ELEMENT_DESC desc{ };
					desc.SemanticName      = cmp.semanticName.c_str();
					desc.AlignedByteOffset = descriptors.empty() ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
					desc.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

					switch( cmp.type )
					{
						default:                      desc.Format = DXGI_FORMAT_UNKNOWN;            break;
						case vertex_component::Float: desc.Format = DXGI_FORMAT_R32_FLOAT;          break;
						case vertex_component::Vec2:  desc.Format = DXGI_FORMAT_R32G32_FLOAT;       break;
						case vertex_component::Vec3:  desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;    break;
						case vertex_component::Vec4:  desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
					}

					descriptors.push_back( desc );
				}

				ID3D11Device&      device      = *( std::get_if< __render_context_impl_d3d11 >( render_context::get_current()->get_impl_ptr() )->device );
				ID3D11InputLayout* inputLayout = nullptr;
				device.CreateInputLayout( descriptors.data(), static_cast< UINT >( descriptors.size() ), impl->vertexData->GetBufferPointer(), impl->vertexData->GetBufferSize(), &inputLayout );
				impl->inputLayout.reset( inputLayout );

				break;
			}
		#endif
		}
	}

	void graphics_pipeline::draw( const vertex_buffer& vb )
	{
		switch( m_impl.index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_2_0 > ):
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_3_0 > ):
			{
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();
				functions.draw_arrays( gl::draw_mode::Triangles, 0, static_cast< GLsizei >( vb.get_count() ) );
				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_d3d11 > ):
			{
				ID3D11DeviceContext& deviceContext = *( std::get_if< __render_context_impl_d3d11 >( render_context::get_current()->get_impl_ptr() )->deviceContext );
				deviceContext.Draw( static_cast< UINT >( vb.get_count() ), 0 );
				break;
			}
		#endif
		}
	}

	void graphics_pipeline::draw( const index_buffer& ib )
	{
		switch( m_impl.index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_2_0 > ):
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_opengl_3_0 > ):
			{
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				gl::index_type indexType{ };
				switch( ib.get_format() )
				{
					default:
					case index_format::Byte:       indexType = gl::index_type::Byte;  break;
					case index_format::Word:       indexType = gl::index_type::Short; break;
					case index_format::DoubleWord: indexType = gl::index_type::Int;   break;
				}

				functions.draw_elements( gl::draw_mode::Triangles, static_cast< GLsizei >( ib.get_count() ), indexType, nullptr );

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( graphics_pipeline_impl_index_v< __graphics_pipeline_impl_d3d11 > ):
			{
				ID3D11DeviceContext& deviceContext = *( std::get_if< __render_context_impl_d3d11 >( render_context::get_current()->get_impl_ptr() )->deviceContext );
				deviceContext.DrawIndexed( static_cast< UINT >( ib.get_count() ), 0, 0 );
				break;
			}
		#endif
		}
	}
}
