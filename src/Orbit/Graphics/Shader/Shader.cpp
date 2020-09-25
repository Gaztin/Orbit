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

			// Create vertex shader
			ComPtr< ID3DBlob > vertex_data;
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

			// Create pixel shader
			ComPtr< ID3DBlob > pixel_data;
			{
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

			// Reflect vertex shader to extract constant buffer members
			{
				ComPtr< ID3D11ShaderReflection > shader_reflection;

				if( SUCCEEDED( D3DReflect( vertex_data->GetBufferPointer(), vertex_data->GetBufferSize(), __uuidof( ID3D11ShaderReflection ), reinterpret_cast< void** >( &shader_reflection.ptr_ ) ) ) )
				{
					// Get shader descriptor
					D3D11_SHADER_DESC shader_desc;
					shader_reflection->GetDesc( &shader_desc );

					// Iterate all constant buffers in shader
					for( UINT buffer_index = 0; buffer_index < shader_desc.ConstantBuffers; ++buffer_index )
					{
						ID3D11ShaderReflectionConstantBuffer* shader_buffer = shader_reflection->GetConstantBufferByIndex( buffer_index );

						// Get descriptor
						D3D11_SHADER_BUFFER_DESC buffer_desc;
						shader_buffer->GetDesc( &buffer_desc );

						// Create constant CPU buffer
						{
							D3D11_BUFFER_DESC desc;
							desc.ByteWidth           = buffer_desc.Size;
							desc.Usage               = D3D11_USAGE_STAGING;
							desc.BindFlags           = 0;
							desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
							desc.MiscFlags           = 0;
							desc.StructureByteStride = 0;

							ComPtr< ID3D11Buffer > buffer;
							if( SUCCEEDED( d3d11.device->CreateBuffer( &desc, nullptr, &buffer.ptr_ ) ) )
							{
								details.vertex_constant_cpu_buffers.emplace_back( std::move( buffer ) );
							}
						}

						// Create constant GPU buffer
						{
							D3D11_BUFFER_DESC desc;
							desc.ByteWidth           = buffer_desc.Size;
							desc.Usage               = D3D11_USAGE_DYNAMIC;
							desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
							desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
							desc.MiscFlags           = 0;
							desc.StructureByteStride = 0;

							ComPtr< ID3D11Buffer > buffer;
							if( SUCCEEDED( d3d11.device->CreateBuffer( &desc, nullptr, &buffer.ptr_ ) ) )
							{
								details.vertex_constant_gpu_buffers.emplace_back( std::move( buffer ) );
							}
						}

						// Iterate variables
						for( UINT variable_index = 0; variable_index < buffer_desc.Variables; ++variable_index )
						{
							ID3D11ShaderReflectionVariable* variable = shader_buffer->GetVariableByIndex( variable_index );

							// Get variable descriptor
							D3D11_SHADER_VARIABLE_DESC variable_desc;
							variable->GetDesc( &variable_desc );

							// Register uniform
							Uniform uniform;
							uniform.name         = variable_desc.Name;
							uniform.buffer_index = buffer_index;
							uniform.size         = variable_desc.Size;
							uniform.offset       = variable_desc.StartOffset;
							vertex_uniforms_.emplace_back( std::move( uniform ) );
						}
					}
				}
			}

			// Reflect pixel shader to extract constant buffer members
			{
				ComPtr< ID3D11ShaderReflection > shader_reflection;

				if( SUCCEEDED( D3DReflect( pixel_data->GetBufferPointer(), pixel_data->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast< void** >( &shader_reflection.ptr_ ) ) ) )
				{
					// Get shader descriptor
					D3D11_SHADER_DESC shader_desc;
					shader_reflection->GetDesc( &shader_desc );

					// Iterate all constant buffers in shader
					for( UINT buffer_index = 0; buffer_index < shader_desc.ConstantBuffers; ++buffer_index )
					{
						ID3D11ShaderReflectionConstantBuffer* shader_buffer = shader_reflection->GetConstantBufferByIndex( buffer_index );

						// Get descriptor
						D3D11_SHADER_BUFFER_DESC buffer_desc;
						shader_buffer->GetDesc( &buffer_desc );

						// Create constant CPU buffer
						{
							D3D11_BUFFER_DESC desc;
							desc.ByteWidth           = buffer_desc.Size;
							desc.Usage               = D3D11_USAGE_STAGING;
							desc.BindFlags           = 0;
							desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
							desc.MiscFlags           = 0;
							desc.StructureByteStride = 0;

							ComPtr< ID3D11Buffer > buffer;
							if( SUCCEEDED( d3d11.device->CreateBuffer( &desc, nullptr, &buffer.ptr_ ) ) )
							{
								details.pixel_constant_cpu_buffers.emplace_back( std::move( buffer ) );
							}
						}

						// Create constant GPU buffer
						{
							D3D11_BUFFER_DESC desc;
							desc.ByteWidth           = buffer_desc.Size;
							desc.Usage               = D3D11_USAGE_DYNAMIC;
							desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
							desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
							desc.MiscFlags           = 0;
							desc.StructureByteStride = 0;

							ComPtr< ID3D11Buffer > buffer;
							if( SUCCEEDED( d3d11.device->CreateBuffer( &desc, nullptr, &buffer.ptr_ ) ) )
							{
								details.pixel_constant_gpu_buffers.emplace_back( std::move( buffer ) );
							}
						}

						// Iterate variables
						for( UINT variable_index = 0; variable_index < buffer_desc.Variables; ++variable_index )
						{
							ID3D11ShaderReflectionVariable* variable = shader_buffer->GetVariableByIndex( variable_index );

							// Get variable descriptor
							D3D11_SHADER_VARIABLE_DESC variable_desc;
							variable->GetDesc( &variable_desc );

							// Register uniform
							Uniform uniform;
							uniform.name         = variable_desc.Name;
							uniform.buffer_index = buffer_index;
							uniform.size         = variable_desc.Size;
							uniform.offset       = variable_desc.StartOffset;
							pixel_uniforms_.emplace_back( std::move( uniform ) );
						}
					}
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
					desc.SemanticIndex     = component.semantic_index;

					switch( component.type )
					{
						default:                            { assert( false );                    } break;
						case VertexComponent::Position:     { desc.SemanticName = "POSITION";     } break;
						case VertexComponent::Binormal:     { desc.SemanticName = "BINORMAL";     } break;
						case VertexComponent::Tangent:      { desc.SemanticName = "TANGENT";      } break;
						case VertexComponent::Normal:       { desc.SemanticName = "NORMAL";       } break;
						case VertexComponent::Color:        { desc.SemanticName = "COLOR";        } break;
						case VertexComponent::TexCoord:     { desc.SemanticName = "TEXCOORD";     } break;
						case VertexComponent::BlendIndices: { desc.SemanticName = "BLENDINDICES"; } break;
						case VertexComponent::BlendWeights: { desc.SemanticName = "BLENDWEIGHT";  } break;
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

			// Reflect shader to register uniforms and buffers
			{
				if( gl.version.RequireGL( 3, 1 ) || gl.version.RequireGLES( 3 ) )
				{
					// Get number of uniform blocks
					GLint active_uniform_blocks = 0;
					glGetProgramiv( details.program, OpenGLProgramParam::ActiveUniformBlocks, &active_uniform_blocks );

					for( GLint block_index = 0; block_index < active_uniform_blocks; ++block_index )
					{
						GLchar  block_name_buf[ 256 ]         = { };
						GLsizei block_name_length             = 0;
						GLint   data_size                     = 0;
						GLint   referenced_by_vertex_shader   = 0;
						GLint   referenced_by_fragment_shader = 0;
						GLint   active_uniforms               = 0;

						// Get info about the uniform block
						glGetActiveUniformBlockName( details.program, block_index, std::size( block_name_buf ), &block_name_length, block_name_buf );
						glGetActiveUniformBlockiv( details.program, block_index, OpenGLUniformBlockParam::DataSize,                   &data_size );
						glGetActiveUniformBlockiv( details.program, block_index, OpenGLUniformBlockParam::ReferencedByVertexShader,   &referenced_by_vertex_shader );
						glGetActiveUniformBlockiv( details.program, block_index, OpenGLUniformBlockParam::ReferencedByFragmentShader, &referenced_by_fragment_shader );
						glGetActiveUniformBlockiv( details.program, block_index, OpenGLUniformBlockParam::ActiveUniforms,             &active_uniforms );
						assert( referenced_by_vertex_shader ^ referenced_by_fragment_shader );

						// Create buffer
						GLuint buffer = 0;
						glGenBuffers( 1, &buffer );
						glBindBuffer( OpenGLBufferTarget::Uniform, buffer );
						glBufferData( OpenGLBufferTarget::Uniform, data_size, nullptr, OpenGLBufferUsage::StreamDraw );
						glBindBuffer( OpenGLBufferTarget::Uniform, 0 );

						// Register uniform block
						Private::_ShaderDetailsOpenGL::UniformBlock uniform_block;
						uniform_block.buffer                        = buffer;
						uniform_block.referenced_by_vertex_shader   = referenced_by_vertex_shader;
						uniform_block.referenced_by_fragment_shader = referenced_by_fragment_shader;
						details.uniform_blocks.push_back( uniform_block );

						// Get uniform indices
						auto active_uniform_indices = std::unique_ptr< GLint[] >( new GLint[ active_uniforms ] );
						glGetActiveUniformBlockiv( details.program, block_index, OpenGLUniformBlockParam::ActiveUniformIndices, active_uniform_indices.get() );

						// Iterate the variables
						GLint uniform_offset = 0;
						for( GLint uniform_index = 0; uniform_index < active_uniforms; ++uniform_index )
						{
							GLchar                uniform_name_buf[ 256 ] = { };
							GLsizei               uniform_name_length     = 0;
							GLint                 uniform_size            = 0;
							OpenGLUniformDataType uniform_type;

							// Get uniform information
							glGetActiveUniform( details.program, active_uniform_indices[ uniform_index ], std::size( uniform_name_buf ), &uniform_name_length, &uniform_size, &uniform_type, uniform_name_buf );

							// Magnify uniform size based on type
							switch( uniform_type )
							{
								case OpenGLUniformDataType::Float:           { uniform_size *= ( sizeof( float )        * 1   ); } break;
								case OpenGLUniformDataType::FloatVec2:       { uniform_size *= ( sizeof( float )        * 2   ); } break;
								case OpenGLUniformDataType::FloatVec3:       { uniform_size *= ( sizeof( float )        * 3   ); } break;
								case OpenGLUniformDataType::FloatVec4:       { uniform_size *= ( sizeof( float )        * 4   ); } break;
								case OpenGLUniformDataType::Double:          { uniform_size *= ( sizeof( double )       * 1   ); } break;
								case OpenGLUniformDataType::DoubleVec2:      { uniform_size *= ( sizeof( double )       * 2   ); } break;
								case OpenGLUniformDataType::DoubleVec3:      { uniform_size *= ( sizeof( double )       * 3   ); } break;
								case OpenGLUniformDataType::DoubleVec4:      { uniform_size *= ( sizeof( double )       * 4   ); } break;
								case OpenGLUniformDataType::Int:             { uniform_size *= ( sizeof( int )          * 1   ); } break;
								case OpenGLUniformDataType::IntVec2:         { uniform_size *= ( sizeof( int )          * 2   ); } break;
								case OpenGLUniformDataType::IntVec3:         { uniform_size *= ( sizeof( int )          * 3   ); } break;
								case OpenGLUniformDataType::IntVec4:         { uniform_size *= ( sizeof( int )          * 4   ); } break;
								case OpenGLUniformDataType::UnsignedInt:     { uniform_size *= ( sizeof( unsigned int ) * 1   ); } break;
								case OpenGLUniformDataType::UnsignedIntVec2: { uniform_size *= ( sizeof( unsigned int ) * 2   ); } break;
								case OpenGLUniformDataType::UnsignedIntVec3: { uniform_size *= ( sizeof( unsigned int ) * 3   ); } break;
								case OpenGLUniformDataType::UnsignedIntVec4: { uniform_size *= ( sizeof( unsigned int ) * 4   ); } break;
								case OpenGLUniformDataType::Bool:            { uniform_size *= ( sizeof( bool )         * 1   ); } break;
								case OpenGLUniformDataType::BoolVec2:        { uniform_size *= ( sizeof( bool )         * 2   ); } break;
								case OpenGLUniformDataType::BoolVec3:        { uniform_size *= ( sizeof( bool )         * 3   ); } break;
								case OpenGLUniformDataType::BoolVec4:        { uniform_size *= ( sizeof( bool )         * 4   ); } break;
								case OpenGLUniformDataType::FloatMat2:       { uniform_size *= ( sizeof( float )        * 2*2 ); } break;
								case OpenGLUniformDataType::FloatMat3:       { uniform_size *= ( sizeof( float )        * 3*3 ); } break;
								case OpenGLUniformDataType::FloatMat4:       { uniform_size *= ( sizeof( float )        * 4*4 ); } break;
								case OpenGLUniformDataType::FloatMat2x3:     { uniform_size *= ( sizeof( float )        * 2*3 ); } break;
								case OpenGLUniformDataType::FloatMat2x4:     { uniform_size *= ( sizeof( float )        * 2*4 ); } break;
								case OpenGLUniformDataType::FloatMat3x2:     { uniform_size *= ( sizeof( float )        * 3*2 ); } break;
								case OpenGLUniformDataType::FloatMat3x4:     { uniform_size *= ( sizeof( float )        * 3*4 ); } break;
								case OpenGLUniformDataType::FloatMat4x2:     { uniform_size *= ( sizeof( float )        * 4*2 ); } break;
								case OpenGLUniformDataType::FloatMat4x3:     { uniform_size *= ( sizeof( float )        * 4*3 ); } break;
								case OpenGLUniformDataType::DoubleMat2:      { uniform_size *= ( sizeof( double )       * 2*2 ); } break;
								case OpenGLUniformDataType::DoubleMat3:      { uniform_size *= ( sizeof( double )       * 3*3 ); } break;
								case OpenGLUniformDataType::DoubleMat4:      { uniform_size *= ( sizeof( double )       * 4*4 ); } break;
								case OpenGLUniformDataType::DoubleMat2x3:    { uniform_size *= ( sizeof( double )       * 2*3 ); } break;
								case OpenGLUniformDataType::DoubleMat2x4:    { uniform_size *= ( sizeof( double )       * 2*4 ); } break;
								case OpenGLUniformDataType::DoubleMat3x2:    { uniform_size *= ( sizeof( double )       * 3*2 ); } break;
								case OpenGLUniformDataType::DoubleMat3x4:    { uniform_size *= ( sizeof( double )       * 3*4 ); } break;
								case OpenGLUniformDataType::DoubleMat4x2:    { uniform_size *= ( sizeof( double )       * 4*2 ); } break;
								case OpenGLUniformDataType::DoubleMat4x3:    { uniform_size *= ( sizeof( double )       * 4*3 ); } break;
								default:                                     { uniform_size  = 0;                                } break;
							}

							// Register uniform
							Uniform uniform;
							uniform.name   = uniform_name_buf;
							uniform.offset = uniform_offset;
							uniform.size   = uniform_size;

							if( referenced_by_vertex_shader )
							{
								uniform.buffer_index = block_index;
								vertex_uniforms_.push_back( uniform );
							}

							if( referenced_by_fragment_shader )
							{
								uniform.buffer_index = block_index;
								pixel_uniforms_.push_back( uniform );
							}

							// Increment offset
							uniform_offset += uniform_size;
						}
					}
				}
				else
				{
					// Get active uniforms
					GLint active_uniforms = 0;
					glGetProgramiv( details.program, OpenGLProgramParam::ActiveUniforms, &active_uniforms );

					// Iterate the variables
					GLint uniform_offset = 0;
					for( GLint uniform_index = 0; uniform_index < active_uniforms; ++uniform_index )
					{
						GLchar                uniform_name_buf[ 256 ] = { };
						GLsizei               uniform_name_length     = 0;
						GLint                 uniform_size            = 0;
						OpenGLUniformDataType uniform_type;

						// Get uniform information
						glGetActiveUniform( details.program, uniform_index, std::size( uniform_name_buf ), &uniform_name_length, &uniform_size, &uniform_type, uniform_name_buf	);

						// Register uniform
						Uniform uniform;
						uniform.name         = uniform_name_buf;
						uniform.buffer_index = 0;
						uniform.offset       = uniform_offset;
						uniform.size         = uniform_size;
						vertex_uniforms_.push_back( uniform );
						pixel_uniforms_.push_back( uniform );

						// Increment offset
						uniform_offset += uniform_size;
					}
				}
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

		for( auto& uniform_block : details.uniform_blocks )
			glDeleteBuffers( 1, &uniform_block.buffer );

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

			for( size_t i = 0; i < details.vertex_constant_gpu_buffers.size(); ++i )
			{
				d3d11.device_context->CopyResource( details.vertex_constant_gpu_buffers[ i ].ptr_, details.vertex_constant_cpu_buffers[ i ].ptr_ );
				d3d11.device_context->VSSetConstantBuffers( i, 1, &details.vertex_constant_gpu_buffers[ i ].ptr_ );
			}

			for( size_t i = 0; i < details.pixel_constant_gpu_buffers.size(); ++i )
			{
				d3d11.device_context->CopyResource( details.pixel_constant_gpu_buffers[ i ].ptr_, details.pixel_constant_cpu_buffers[ i ].ptr_ );
				d3d11.device_context->PSSetConstantBuffers( i, 1, &details.pixel_constant_gpu_buffers[ i ].ptr_ );
			}

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
				glEnableVertexAttribArray( static_cast< GLuint >( component.layout_index ) );

				switch( component.GetDataType() )
				{
					case PrimitiveDataType::Float:
					{
						glVertexAttribPointer( static_cast< GLuint >( component.layout_index ), static_cast< GLint >( component.GetDataCount() ), OpenGLVertexAttribDataType::Float, GL_FALSE, static_cast< GLsizei >( details.layout.GetStride() ), ptr );
					} break;

					case PrimitiveDataType::Int:
					{
						glVertexAttribIPointer( static_cast< GLuint >( component.layout_index ), static_cast< GLint >( component.GetDataCount() ), OpenGLVertexAttribDataType::Int, static_cast< GLsizei >( details.layout.GetStride() ), ptr );
					} break;
				}

				ptr += component.GetSize();
			}

			for( size_t i = 0; i < details.uniform_blocks.size(); ++i )
			{
				auto& uniform_block = details.uniform_blocks[ i ];

				glBindBuffer( OpenGLBufferTarget::Uniform, uniform_block.buffer );
				glBindBufferBase( OpenGLBufferTarget::Uniform, i, uniform_block.buffer );
				glUniformBlockBinding( details.program, i, i );
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
				glDisableVertexAttribArray( static_cast< GLuint >( component.layout_index ) );

			glUseProgram( 0 );

			if( details.vao )
				glBindVertexArray( 0 );

			break;
		}

	#endif // ORB_HAS_OPENGL

	}
}

void Shader::SetVertexUniform( std::string_view name, const void* data, size_t size ) const
{
	// Find uniform among registered uniforms
	auto uniform = std::find_if( vertex_uniforms_.begin(), vertex_uniforms_.end(), [ name ]( const Uniform& u ) { return u.name == name; } );
	if( uniform == vertex_uniforms_.end() )
		return;

	assert( uniform->size >= size );

	switch( details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ShaderDetailsD3D11, Private::ShaderDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details = std::get< Private::_ShaderDetailsD3D11 >( details_ );
			auto& buffer  = details.vertex_constant_cpu_buffers[ uniform->buffer_index ];

			D3D11_MAPPED_SUBRESOURCE subresource;
			if( SUCCEEDED( d3d11.device_context->Map( buffer.ptr_, 0, D3D11_MAP_WRITE, 0, &subresource ) ) )
			{
				assert( subresource.RowPitch >= uniform->size );

				memcpy( static_cast< uint8_t* >( subresource.pData ) + uniform->offset, data, size );

				d3d11.device_context->Unmap( buffer.ptr_, 0 );
			}

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > ):
		{
			auto& gl      = std::get< Private::_RenderContextDetailsOpenGL >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details = std::get< Private::_ShaderDetailsOpenGL >( details_ );

			if( gl.version.RequireGL( 3, 1 ) || gl.version.RequireGLES( 3 ) )
			{
				auto& uniform_block = details.uniform_blocks[ uniform->buffer_index ];

				glBindBuffer( OpenGLBufferTarget::Uniform, uniform_block.buffer );

				void* dst = glMapBufferRange( OpenGLBufferTarget::Uniform, uniform->offset, uniform->size, OpenGLMapAccess::WriteBit );

				memcpy( dst, data, size );

				glUnmapBuffer( OpenGLBufferTarget::Uniform );
				glBindBuffer( OpenGLBufferTarget::Uniform, 0 );
			}
			else
			{
				const GLint location = glGetUniformLocation( details.program, name.data() );

				// #TODO: Not sure how to implement this in older versions of OpenGL. Would need to keep track of uniform type and such.
				glUniform1fv( location, size / sizeof( float ), static_cast< const float* >( data ) );
			}

		} break;

	#endif // ORB_HAS_OPENGL

	}
}

void Shader::SetPixelUniform( std::string_view name, const void* data, size_t size ) const
{
	// Find uniform among registered uniforms
	auto uniform = std::find_if( pixel_uniforms_.begin(), pixel_uniforms_.end(), [ name ]( const Uniform& u ) { return u.name == name; } );
	if( uniform == pixel_uniforms_.end() )
		return;

	assert( uniform->size >= size );

	switch( details_.index() )
	{

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_ShaderDetailsD3D11, Private::ShaderDetails > ):
		{
			auto& d3d11   = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details = std::get< Private::_ShaderDetailsD3D11 >( details_ );
			auto& buffer  = details.pixel_constant_cpu_buffers[ uniform->buffer_index ];

			D3D11_MAPPED_SUBRESOURCE subresource;
			if( SUCCEEDED( d3d11.device_context->Map( buffer.ptr_, 0, D3D11_MAP_WRITE, 0, &subresource ) ) )
			{
				assert( subresource.RowPitch >= uniform->size );

				memcpy( static_cast< uint8_t* >( subresource.pData ) + uniform->offset, data, size );

				d3d11.device_context->Unmap( buffer.ptr_, 0 );
			}

		} break;

	#endif // ORB_HAS_D3D11
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > ):
		{
			auto& gl      = std::get< Private::_RenderContextDetailsOpenGL >( RenderContext::GetInstance().GetPrivateDetails() );
			auto& details = std::get< Private::_ShaderDetailsOpenGL >( details_ );

			if( gl.version.RequireGL( 3, 1 ) || gl.version.RequireGLES( 3 ) )
			{
				auto& uniform_block = details.uniform_blocks[ uniform->buffer_index ];

				glBindBuffer( OpenGLBufferTarget::Uniform, uniform_block.buffer );

				void* dst = glMapBufferRange( OpenGLBufferTarget::Uniform, uniform->offset, uniform->size, OpenGLMapAccess::WriteBit );

				memcpy( dst, data, size );

				glUnmapBuffer( OpenGLBufferTarget::Uniform );
			}
			else
			{
				const GLint location = glGetUniformLocation( details.program, name.data() );

				// #TODO: Not sure how to implement this in older versions of OpenGL. Would need to keep track of uniform type and such.
				glUniform1fv( location, size / sizeof( float ), static_cast< const float* >( data ) );
			}

		} break;

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
