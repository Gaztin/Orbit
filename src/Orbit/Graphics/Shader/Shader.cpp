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

#include "Shader.h"

#include <array>

#include "Orbit/Core/IO/Log.h"
#include "Orbit/Graphics/API/OpenGL/GLSL.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Context/RenderContext.h"

#if( ORB_HAS_D3D11 )
#  include <d3dcompiler.h>
#endif

ORB_NAMESPACE_BEGIN

#if( ORB_HAS_OPENGL )
GLuint CompileGLSL( std::string_view source, ShaderType shader_type, OpenGLShaderType gl_shader_type );
#endif

Shader::Shader( std::string_view source, const VertexLayout& vertex_layout )
{
	auto& context_details = RenderContext::Get().GetPrivateDetails();

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );
			auto& details = m_details.emplace< Private::_ShaderDetailsD3D11 >();

			UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS;

		#if defined( _DEBUG )
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION );
		#else
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL3 );
		#endif

			const D3D_SHADER_MACRO macros[]
			{
				"HLSL",  "1",
				nullptr, nullptr,
			};

			ID3DBlob* vertex_data = nullptr;

			/* Create vertex shader */
			{
				ID3DBlob* errors = nullptr;

				if( FAILED( D3DCompile( source.data(), source.size(), nullptr, macros, nullptr, "VSMain", "vs_5_0", flags, 0, &vertex_data, &errors ) ) )
				{
					LogError( "%s", errors->GetBufferPointer() );
					errors->Release();
				}

				ID3D11VertexShader* vertex_shader = nullptr;

				if( SUCCEEDED( d3d11.device->CreateVertexShader( vertex_data->GetBufferPointer(), vertex_data->GetBufferSize(), nullptr, &vertex_shader ) ) )
				{
					details.vertex_shader.reset( vertex_shader );
				}
			}

			/* Create pixel shader */
			{
				ID3DBlob* pixel_data;
				ID3DBlob* errors;

				if( FAILED( D3DCompile( source.data(), source.size(), nullptr, macros, nullptr, "PSMain", "ps_5_0", flags, 0, &pixel_data, &errors ) ) )
				{
					LogError( "%s", errors->GetBufferPointer() );
					errors->Release();
				}

				ID3D11PixelShader* pixel_shader;

				if( SUCCEEDED( d3d11.device->CreatePixelShader( pixel_data->GetBufferPointer(), pixel_data->GetBufferSize(), nullptr, &pixel_shader ) ) )
				{
					pixel_data->Release();
					details.pixel_shader.reset( pixel_shader );
				}
			}

			/* Create input layout */
			if( vertex_data )
			{
				std::vector< D3D11_INPUT_ELEMENT_DESC > descriptors;

				for( const VertexComponent& component : vertex_layout )
				{
					D3D11_INPUT_ELEMENT_DESC desc { };
					desc.SemanticName      = component.semantic_name.c_str();
					desc.AlignedByteOffset = descriptors.empty() ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
					desc.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

					switch( component.type )
					{
						default:                     { desc.Format = DXGI_FORMAT_UNKNOWN;            } break;
						case VertexComponent::Float: { desc.Format = DXGI_FORMAT_R32_FLOAT;          } break;
						case VertexComponent::Vec2:  { desc.Format = DXGI_FORMAT_R32G32_FLOAT;       } break;
						case VertexComponent::Vec3:  { desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;    } break;
						case VertexComponent::Vec4:  { desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; } break;
					}

					descriptors.push_back( desc );
				}

				ID3D11InputLayout* input_layout;

				if( SUCCEEDED( d3d11.device->CreateInputLayout( descriptors.data(), static_cast< UINT >( descriptors.size() ), vertex_data->GetBufferPointer(), vertex_data->GetBufferSize(), &input_layout ) ) )
				{
					details.input_layout.reset( input_layout );
				}

				vertex_data->Release();
			}

			/* Create sampler state */
			{
				D3D11_SAMPLER_DESC desc { };
				desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				desc.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.MaxAnisotropy    = 1;
				desc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
				desc.MinLOD           = 0.0f;
				desc.MaxLOD           = D3D11_FLOAT32_MAX;

				ID3D11SamplerState* sampler_state;

				if( SUCCEEDED( d3d11.device->CreateSamplerState( &desc, &sampler_state ) ) )
				{
					details.sampler_state.reset( sampler_state );
				}
			}

			break;
		}

	#endif
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& gl      = std::get< Private::_RenderContextDetailsOpenGL >( context_details );
			auto& details = m_details.emplace< Private::_ShaderDetailsOpenGL >();

			/* Create shader program */
			{
				GLuint vertex_shader   = CompileGLSL( source, ShaderType::Vertex, OpenGLShaderType::Vertex );
				GLuint fragment_shader = CompileGLSL( source, ShaderType::Fragment, OpenGLShaderType::Fragment );

				details.program = glCreateProgram();
				glAttachShader( details.program, vertex_shader );
				glAttachShader( details.program, fragment_shader );
				glLinkProgram( details.program );

				GLint status;
				glGetProgramiv( details.program, OpenGLProgramParam::LinkStatus, &status );
				if( status == GL_FALSE )
				{
					GLint loglen;
					glGetProgramiv( details.program, OpenGLProgramParam::InfoLogLength, &loglen );
					if( loglen > 0 )
					{
						std::string logbuf( static_cast< size_t >( loglen ), '\0' );
						glGetProgramInfoLog( details.program, loglen, nullptr, &logbuf[ 0 ] );
						LogErrorString( logbuf );
					}
				}

				glDeleteShader( fragment_shader );
				glDeleteShader( vertex_shader );
			}

			/* Create vertex array for GL 3.0+ or GLES 3+ */
			if( gl.version.RequireGL( 3, 0 ) || gl.version.RequireGLES( 3 ) )
			{
				glGenVertexArrays( 1, &details.vao );
			}
			else
			{
				details.vao = 0;
			}

			/* Copy vertex layout*/
			details.layout = vertex_layout;

			/* Calculate vertex stride */
			details.vertex_stride = 0;
			for( const VertexComponent& component : vertex_layout )
			{
				switch( component.type )
				{
					case VertexComponent::Float: { details.vertex_stride += sizeof( float );     } break;
					case VertexComponent::Vec2:  { details.vertex_stride += sizeof( float ) * 2; } break;
					case VertexComponent::Vec3:  { details.vertex_stride += sizeof( float ) * 3; } break;
					case VertexComponent::Vec4:  { details.vertex_stride += sizeof( float ) * 4; } break;
				}
			}

			break;
		}

	#endif

	}
}

Shader::~Shader( void )
{

#if( ORB_HAS_OPENGL )

	if( m_details.index() == unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > )
	{
		auto& details = std::get< Private::_ShaderDetailsOpenGL >( m_details );

		if( details.vao )
		{
			glDeleteVertexArrays( 1, &details.vao );
		}

		glDeleteProgram( details.program );
	}

#endif

}

void Shader::Bind( void )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ShaderDetailsD3D11, Private::ShaderDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::Get().GetPrivateDetails() );
			auto& details = std::get< Private::_ShaderDetailsD3D11 >( m_details );

			if( details.input_layout )
				d3d11.device_context->IASetInputLayout( details.input_layout.get() );

			if( details.vertex_shader )
				d3d11.device_context->VSSetShader( details.vertex_shader.get(), nullptr, 0 );

			if( details.pixel_shader )
				d3d11.device_context->PSSetShader( details.pixel_shader.get(), nullptr, 0 );

			if( details.sampler_state )
			{
				ID3D11SamplerState* sampler_states[] = { details.sampler_state.get() };
				d3d11.device_context->PSSetSamplers( 0, 1, sampler_states );
			}

			break;
		}

	#endif
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > ):
		{
			auto& details = std::get< Private::_ShaderDetailsOpenGL >( m_details );

			glBindVertexArray( details.vao );
			glUseProgram( details.program );

			const uint8_t* ptr = nullptr;

			for( GLuint i = 0; i < details.layout.size(); ++i )
			{
				OpenGLVertexAttribDataType data_type { };
				GLint                      data_length = 0;

				switch( details.layout[ i ].type )
				{
					default:
					{
						LogError( "Invalid attrib data type" );
						continue;
					}

					case VertexComponent::Float:
					{
						data_type   = OpenGLVertexAttribDataType::Float;
						data_length = 1;
						break;
					}

					case VertexComponent::Vec2:
					{
						data_type   = OpenGLVertexAttribDataType::Float;
						data_length = 2;
						break;
					}

					case VertexComponent::Vec3:
					{
						data_type   = OpenGLVertexAttribDataType::Float;
						data_length = 3;
						break;
					}

					case VertexComponent::Vec4:
					{
						data_type   = OpenGLVertexAttribDataType::Float;
						data_length = 4;
						break;
					}
				}

				glEnableVertexAttribArray( i );
				glVertexAttribPointer( i, data_length, data_type, GL_FALSE, details.vertex_stride, ptr );

				switch( details.layout[ i ].type )
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

	}
}

void Shader::Unbind( void )
{
	switch( m_details.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ShaderDetailsD3D11, Private::ShaderDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::Get().GetPrivateDetails() );
			auto& details = std::get< Private::_ShaderDetailsD3D11 >( m_details );

			if( details.input_layout )
			{
				ID3D11InputLayout* bound_input_layout;
				d3d11.device_context->IAGetInputLayout( &bound_input_layout );
				if( bound_input_layout == details.input_layout.get() )
				{
					d3d11.device_context->IASetInputLayout( nullptr );
					bound_input_layout->Release();
				}
			}

			if( details.vertex_shader )
			{
				ID3D11VertexShader* bound_vertex_shader;
				d3d11.device_context->VSGetShader( &bound_vertex_shader, nullptr, nullptr );
				if( bound_vertex_shader == details.vertex_shader.get() )
				{
					d3d11.device_context->VSSetShader( nullptr, nullptr, 0 );
					bound_vertex_shader->Release();
				}
			}

			if( details.pixel_shader )
			{
				ID3D11PixelShader* bound_pixel_shader;
				d3d11.device_context->PSGetShader( &bound_pixel_shader, nullptr, nullptr );
				if( bound_pixel_shader == details.pixel_shader.get() )
				{
					d3d11.device_context->PSSetShader( nullptr, nullptr, 0 );
					bound_pixel_shader->Release();
				}
			}

			if( details.sampler_state )
			{
				ID3D11SamplerState* bound_sampler_state;
				d3d11.device_context->PSGetSamplers( 0, 1, &bound_sampler_state );
				if( bound_sampler_state == details.sampler_state.get() )
				{
					d3d11.device_context->PSSetSamplers( 0, 0, nullptr );
					bound_sampler_state->Release();
				}
			}

			break;
		}

	#endif
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > ):
		{
			auto& details = std::get< Private::_ShaderDetailsOpenGL >( m_details );

			for( GLuint i = 0; i < details.layout.size(); ++i )
				glDisableVertexAttribArray( i );

			glUseProgram( 0 );

			if( details.vao )
			{
				glBindVertexArray( 0 );
			}

			break;
		}

	#endif

	}
}

#if( ORB_HAS_OPENGL )

GLuint CompileGLSL( std::string_view source, ShaderType shader_type, OpenGLShaderType gl_shader_type )
{
	auto& gl = std::get< Private::_RenderContextDetailsOpenGL >( RenderContext::Get().GetPrivateDetails() );

	const std::string_view version_directive  = GLSL::GetVersionDirective( gl.version );
	const std::string_view glsl_define        = GLSL::GetGLSLDefine();
	const std::string_view shader_type_define = GLSL::GetShaderTypeDefine( shader_type );
	const std::string_view precision          = GLSL::GetPrecision( gl.version );
	const std::string_view constants_macros   = GLSL::GetConstantsMacros( gl.version );
	const std::string_view varying_macro      = GLSL::GetVaryingMacro( gl.version, shader_type );
	const std::string_view attribute_macro    = GLSL::GetAttributeMacro( gl.version, shader_type );
	const std::string_view out_color_macro    = GLSL::GetOutColorMacro( gl.version );

	const std::array< const GLchar*, 9 > sources
	{
		version_directive.data(),
		glsl_define.data(),
		shader_type_define.data(),
		precision.data(),
		constants_macros.data(),
		varying_macro.data(),
		attribute_macro.data(),
		out_color_macro.data(),
		reinterpret_cast< const GLchar* >( source.data() ),
	};

	const std::array< GLint, 9 > lengths
	{
		static_cast< GLint >( version_directive.size() ),
		static_cast< GLint >( glsl_define.size() ),
		static_cast< GLint >( shader_type_define.size() ),
		static_cast< GLint >( precision.size() ),
		static_cast< GLint >( constants_macros.size() ),
		static_cast< GLint >( varying_macro.size() ),
		static_cast< GLint >( attribute_macro.size() ),
		static_cast< GLint >( out_color_macro.size() ),
		static_cast< GLint >( source.size() ),
	};

	GLuint shader = glCreateShader( gl_shader_type );
	glShaderSource( shader, static_cast< GLsizei >( sources.size() ), sources.data(), lengths.data() );
	glCompileShader( shader );

	GLint status;
	glGetShaderiv( shader, OpenGLShaderParam::CompileStatus, &status );
	if( status == GL_FALSE )
	{
		GLint loglen;
		glGetShaderiv( shader, OpenGLShaderParam::InfoLogLength, &loglen );
		if( loglen > 0 )
		{
			std::string logbuf( static_cast< size_t >( loglen ), '\0' );
			glGetShaderInfoLog( shader, loglen, nullptr, &logbuf[ 0 ] );
			LogErrorString( logbuf );
		}

		return 0;
	}

	return shader;
}

#endif

ORB_NAMESPACE_END
