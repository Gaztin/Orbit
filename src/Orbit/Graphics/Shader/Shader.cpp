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

#include "Shader.h"

#include "Orbit/Core/IO/Log.h"
#include "Orbit/Graphics/API/OpenGL/GLSL.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Context/RenderContext.h"

#include <array>

#if( ORB_HAS_D3D11 )
#  include <d3dcompiler.h>
#endif // ORB_HAS_D3D11

ORB_NAMESPACE_BEGIN

#if( ORB_HAS_OPENGL )
GLuint CompileGLSL( std::string_view source, ShaderType shader_type, OpenGLShaderType gl_shader_type );
#endif // ORB_HAS_OPENGL

Shader::Shader( std::string_view source, const VertexLayout& vertex_layout )
{
	auto& context_details = RenderContext::GetInstance().GetPrivateDetails();

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( context_details );
			auto& details = details_.emplace< Private::_ShaderDetailsD3D11 >();

			UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS;

		#if defined( _DEBUG )
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION );
		#else // _DEBUG
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL3 );
		#endif // !_DEBUG

			const D3D_SHADER_MACRO macros[]
			{
				"HLSL",  "1",
				nullptr, nullptr,
			};

			ComPtr< ID3DBlob > vertex_data;

			/* Create vertex shader */
			{
				ComPtr< ID3DBlob > errors;

				if( FAILED( D3DCompile( source.data(), source.size(), nullptr, macros, nullptr, "VSMain", "vs_5_0", flags, 0, &vertex_data.ptr_, &errors.ptr_ ) ) )
				{
					LogError( "%s", errors->GetBufferPointer() );
				}
				else
				{
					d3d11.device->CreateVertexShader( vertex_data->GetBufferPointer(), vertex_data->GetBufferSize(), nullptr, &details.vertex_shader.ptr_ );
				}
			}

			/* Create pixel shader */
			{
				ComPtr< ID3DBlob > pixel_data;
				ComPtr< ID3DBlob > errors;

				if( FAILED( D3DCompile( source.data(), source.size(), nullptr, macros, nullptr, "PSMain", "ps_5_0", flags, 0, &pixel_data.ptr_, &errors.ptr_ ) ) )
				{
					LogError( "%s", errors->GetBufferPointer() );
				}
				else
				{
					d3d11.device->CreatePixelShader( pixel_data->GetBufferPointer(), pixel_data->GetBufferSize(), nullptr, &details.pixel_shader.ptr_ );
				}
			}

			/* Create input layout */
			if( vertex_data )
			{
				std::vector< D3D11_INPUT_ELEMENT_DESC > descriptors;

				for( IndexedVertexComponent component : vertex_layout )
				{
					D3D11_INPUT_ELEMENT_DESC desc { };
					desc.AlignedByteOffset = descriptors.empty() ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
					desc.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

					switch( component.type )
					{
						default:                        { assert( false );                } break;
						case VertexComponent::Position: { desc.SemanticName = "POSITION"; } break;
						case VertexComponent::Normal:   { desc.SemanticName = "NORMAL";   } break;
						case VertexComponent::Color:    { desc.SemanticName = "COLOR";    } break;
						case VertexComponent::TexCoord: { desc.SemanticName = "TEXCOORD"; } break;
						case VertexComponent::JointIDs: { desc.SemanticName = "JOINTIDS"; } break;
						case VertexComponent::Weights:  { desc.SemanticName = "WEIGHTS";  } break;
					}

					switch( component.GetDataType() )
					{
						case PrimitiveDataType::Float:
						{
							switch( component.GetDataCount() )
							{
								default: { assert( false );                              } break;
								case 1:  { desc.Format = DXGI_FORMAT_R32_FLOAT;          } break;
								case 2:  { desc.Format = DXGI_FORMAT_R32G32_FLOAT;       } break;
								case 3:  { desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;    } break;
								case 4:  { desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; } break;
							}
						} break;

						case PrimitiveDataType::Int:
						{
							switch( component.GetDataCount() )
							{
								default: { assert( false );                             } break;
								case 1:  { desc.Format = DXGI_FORMAT_R32_SINT;          } break;
								case 2:  { desc.Format = DXGI_FORMAT_R32G32_SINT;       } break;
								case 3:  { desc.Format = DXGI_FORMAT_R32G32B32_SINT;    } break;
								case 4:  { desc.Format = DXGI_FORMAT_R32G32B32A32_SINT; } break;
							}
						} break;
					}

					descriptors.push_back( desc );
				}

				d3d11.device->CreateInputLayout( descriptors.data(), static_cast< UINT >( descriptors.size() ), vertex_data->GetBufferPointer(), vertex_data->GetBufferSize(), &details.input_layout.ptr_ );
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

				d3d11.device->CreateSamplerState( &desc, &details.sampler_state.ptr_ );
			}

			break;
		}

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			auto& gl      = std::get< Private::_RenderContextDetailsOpenGL >( context_details );
			auto& details = details_.emplace< Private::_ShaderDetailsOpenGL >();

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

			break;
		}

	#endif // ORB_HAS_OPENGL

	}
}

Shader::~Shader( void )
{

#if( ORB_HAS_OPENGL )

	if( details_.index() == unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > )
	{
		auto& details = std::get< Private::_ShaderDetailsOpenGL >( details_ );

		if( details.vao )
			glDeleteVertexArrays( 1, &details.vao );

		glDeleteProgram( details.program );
	}

#endif // ORB_HAS_OPENGL

}

void Shader::Bind( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ShaderDetailsD3D11, Private::ShaderDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details = std::get< Private::_ShaderDetailsD3D11 >( details_ );

			if( details.input_layout )
				d3d11.device_context->IASetInputLayout( details.input_layout.ptr_ );

			if( details.vertex_shader )
				d3d11.device_context->VSSetShader( details.vertex_shader.ptr_, nullptr, 0 );

			if( details.pixel_shader )
				d3d11.device_context->PSSetShader( details.pixel_shader.ptr_, nullptr, 0 );

			if( details.sampler_state )
				d3d11.device_context->PSSetSamplers( 0, 1, &details.sampler_state.ptr_ );

			break;
		}

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > ):
		{
			auto& details = std::get< Private::_ShaderDetailsOpenGL >( details_ );

			glBindVertexArray( details.vao );
			glUseProgram( details.program );

			const uint8_t* ptr = nullptr;

			for( IndexedVertexComponent component : details.layout )
			{
				glEnableVertexAttribArray( static_cast< GLuint >( component.index ) );

				switch( component.GetDataType() )
				{
					case PrimitiveDataType::Float:
					{
						glVertexAttribPointer( static_cast< GLuint >( component.index ), static_cast< GLint >( component.GetDataCount() ), OpenGLVertexAttribDataType::Float, GL_FALSE, static_cast< GLsizei >( details.layout.GetStride() ), ptr );
					} break;

					case PrimitiveDataType::Int:
					{
						glVertexAttribIPointer( static_cast< GLuint >( component.index ), static_cast< GLint >( component.GetDataCount() ), OpenGLVertexAttribDataType::Int, static_cast< GLsizei >( details.layout.GetStride() ), ptr );
					} break;
				}

				ptr += component.GetSize();
			}

			break;
		}

	#endif // ORB_HAS_OPENGL

	}
}

void Shader::Unbind( void )
{
	switch( details_.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ShaderDetailsD3D11, Private::ShaderDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details = std::get< Private::_ShaderDetailsD3D11 >( details_ );

			if( details.input_layout )
			{
				ComPtr< ID3D11InputLayout > bound_input_layout;

				d3d11.device_context->IAGetInputLayout( &bound_input_layout.ptr_ );

				if( bound_input_layout == details.input_layout.ptr_ )
					d3d11.device_context->IASetInputLayout( nullptr );
			}

			if( details.vertex_shader )
			{
				ComPtr< ID3D11VertexShader > bound_vertex_shader;

				d3d11.device_context->VSGetShader( &bound_vertex_shader.ptr_, nullptr, nullptr );

				if( bound_vertex_shader == details.vertex_shader.ptr_ )
					d3d11.device_context->VSSetShader( nullptr, nullptr, 0 );
			}

			if( details.pixel_shader )
			{
				ComPtr< ID3D11PixelShader > bound_pixel_shader;

				d3d11.device_context->PSGetShader( &bound_pixel_shader.ptr_, nullptr, nullptr );

				if( bound_pixel_shader == details.pixel_shader.ptr_ )
					d3d11.device_context->PSSetShader( nullptr, nullptr, 0 );
			}

			if( details.sampler_state )
			{
				ComPtr< ID3D11SamplerState > bound_sampler_state;

				d3d11.device_context->PSGetSamplers( 0, 1, &bound_sampler_state.ptr_ );

				if( bound_sampler_state == details.sampler_state.ptr_ )
					d3d11.device_context->PSSetSamplers( 0, 0, nullptr );
			}

			break;
		}

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > ):
		{
			auto& details = std::get< Private::_ShaderDetailsOpenGL >( details_ );

			for( IndexedVertexComponent component : details.layout )
				glDisableVertexAttribArray( static_cast< GLuint >( component.index ) );

			glUseProgram( 0 );

			if( details.vao )
				glBindVertexArray( 0 );

			break;
		}

	#endif // ORB_HAS_OPENGL

	}
}

#if( ORB_HAS_OPENGL )

GLuint CompileGLSL( std::string_view source, ShaderType shader_type, OpenGLShaderType gl_shader_type )
{
	auto& gl = std::get< Private::_RenderContextDetailsOpenGL >( RenderContext::GetInstance().GetPrivateDetails() );

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

#endif // ORB_HAS_OPENGL

ORB_NAMESPACE_END
