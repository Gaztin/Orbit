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

#include "IGenerator.h"

#include "Orbit/Graphics/Context/RenderContext.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Uniform.h"

#include <cassert>
#include <map>
#include <sstream>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	static IGenerator* current = nullptr;

	IGenerator::IGenerator( void )
	{
		current = this;
	}

	IGenerator::~IGenerator( void )
	{
		if( current == this )
			current = nullptr;
	}

	std::string IGenerator::Generate( void )
	{
		m_source_code.clear();
		m_attribute_prefix.clear();
		m_varying_prefix.clear();

		current = this;

		switch( RenderContext::Get().GetPrivateDetails().index() )
		{

		#if( ORB_HAS_D3D11 )

			case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
			{
				GenerateHLSL();

			} break;

		#endif // ORB_HAS_D3D11
		#if( ORB_HAS_OPENGL )

			case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
			{
				GenerateGLSL();

			} break;

		#endif // ORB_HAS_OPENGL

		}

		return m_source_code;
	}

	VertexLayout IGenerator::GetVertexLayout( void ) const
	{
		return m_attribute_layout;
	}

	void IGenerator::AppendSourceCode( std::string_view code )
	{
		current->m_source_code.append( code );
	}

	ShaderLanguage IGenerator::GetLanguage( void )
	{
		return current->m_language;
	}

	ShaderType IGenerator::GetShaderType( void )
	{
		return current->m_shader_type;
	}

	void IGenerator::IncrementSamplerCount( void )
	{
		++current->m_sampler_count;
	}

	void IGenerator::AddAttribute( VertexComponent component )
	{
		current->m_attribute_layout.Add( component );
	}

	void IGenerator::AddVarying( VertexComponent component )
	{
		current->m_varying_layout.Add( component );
	}

	void IGenerator::AddUniform( UniformBase* uniform )
	{
		current->m_uniforms.push_back( uniform );
	}

	std::string IGenerator::GetAttributePrefix( void )
	{
		return current->m_attribute_prefix;
	}

	std::string IGenerator::GetVaryingPrefix( void )
	{
		return current->m_varying_prefix;
	}

	IVariable IGenerator::Transpose( const IVariable& matrix )
	{
		assert( matrix.GetType() == VariableType::Mat4 );

		matrix.SetUsed();

		return IVariable( "transpose( " + matrix.GetValue() + " )", VariableType::Mat4 );
	}

	IVariable IGenerator::Sample( const IVariable& sampler, const IVariable& texcoord )
	{
		sampler.SetUsed();
		texcoord.SetUsed();

		switch( GetLanguage() )
		{
			case ShaderLanguage::HLSL:
			{
				return IVariable( sampler.GetValue() + ".Sample( default_sampler_state, " + texcoord.GetValue() + " )", VariableType::Vec4 );
			}

			case ShaderLanguage::GLSL:
			{
				return IVariable( "texture( " + sampler.GetValue() + ", " + texcoord.GetValue() + " )", VariableType::Vec4 );
			}
		}

		assert( false );
		return { };
	}

	IVariable IGenerator::Dot( const IVariable& lhs, const IVariable& rhs )
	{
		lhs.SetUsed();
		rhs.SetUsed();

		return IVariable( "dot( " + lhs.GetValue() + ", " + rhs.GetValue() + " )", VariableType::Float );
	}

	void IGenerator::GenerateHLSL( void )
	{
		m_language = ShaderLanguage::HLSL;

		// GLSL type names are canonical
		m_source_code.append( "#define vec2 float2\n" );
		m_source_code.append( "#define vec3 float3\n" );
		m_source_code.append( "#define vec4 float4\n" );
		m_source_code.append( "#define mat4 matrix\n" );

		auto get_vertex_component_type_string = []( size_t data_count ) -> std::string_view
		{
			switch( data_count )
			{
				case 1: { return "float";  }
				case 2: { return "vec2"; }
				case 3: { return "vec3"; }
				case 4: { return "vec4"; }
			}

			assert( false );
			return { };
		};

		auto get_vertex_component_semantic_name = []( VertexComponent component, ShaderType shader_type ) -> std::string_view
		{
			switch( component )
			{
				case VertexComponent::Position: { return shader_type == ShaderType::Fragment ? "SV_POSITION" : "POSITION"; }
				case VertexComponent::Normal:   { return "NORMAL";   }
				case VertexComponent::Color:    { return "COLOR";    }
				case VertexComponent::TexCoord: { return "TEXCOORD"; }
			}

			assert( false );
			return { };
		};

		if( m_sampler_count > 0 )
		{
			std::ostringstream ss;
			ss << "\n";

			for( size_t i = 0; i < m_sampler_count; ++i )
			{
				ss << "Texture2D sampler_";
				ss << i;
				ss << ";\n";
			}

			ss << "\nSamplerState default_sampler_state;\n";

			m_source_code.append( ss.str() );
		}

		size_t uniforms_offset = m_source_code.size();

		m_source_code.append( "\nstruct VertexData\n{\n" );
		for( auto it : m_attribute_layout )
		{
			auto type_string   = get_vertex_component_type_string( it.GetDataCount() );
			auto semantic_name = get_vertex_component_semantic_name( it.type, ShaderType::Vertex );

			std::ostringstream ss;
			ss << "\t" << type_string << " attribute_" << it.index << " : " << semantic_name << ";\n";

			m_source_code.append( ss.str() );
		}
		m_source_code.append( "};\n" );

		m_source_code.append( "\nstruct PixelData\n{\n" );
		for( auto it : m_varying_layout )
		{
			auto type_string   = get_vertex_component_type_string( it.GetDataCount() );
			auto semantic_name = get_vertex_component_semantic_name( it.type, ShaderType::Fragment );

			std::ostringstream ss;
			ss << "\t" << type_string << " varying_" << it.index << " : " << semantic_name << ";\n";

			m_source_code.append( ss.str() );
		}
		m_source_code.append( "};\n" );

		m_source_code.append( "\nPixelData VSMain( VertexData input )\n{\n\tPixelData output;\n" );
		m_shader_type      = ShaderType::Vertex;
		m_attribute_prefix = "input.";
		m_varying_prefix   = "output.";
		auto vs_result     = VSMain();
		m_source_code.append( "\treturn output;\n}\n" );

		{
			std::ostringstream ss;

			for( size_t i = 0; i < m_uniforms.size(); ++i )
			{
				if( m_uniforms[ i ]->m_used )
				{
					ss << "\t" << VariableTypeToString( m_uniforms[ i ]->m_type ) << " uniform_" << i << ";\n";

					/* Reset state for next shader pass */
					m_uniforms[ i ]->m_used = false;
				}
			}

			if( ss.rdbuf()->in_avail() == 0 )
			{
				const std::string what = "\ncbuffer VertexUniforms\n{\n" + ss.str() + "};\n";

				m_source_code.insert( uniforms_offset, what );

				uniforms_offset += what.size();
			}
		}

		m_source_code.append( "\nfloat4 PSMain( PixelData input ) : SV_TARGET\n{\n" );
		m_shader_type      = ShaderType::Fragment;
		m_attribute_prefix = std::string();
		m_varying_prefix   = "input.";
		auto ps_result     = PSMain();
		m_source_code.append( "\treturn " + ps_result.GetValue() + ";\n}\n" );

		{
			std::ostringstream ss;

			for( size_t i = 0; i < m_uniforms.size(); ++i )
			{
				if( m_uniforms[ i ]->m_used )
				{
					ss << "\t" << VariableTypeToString( m_uniforms[ i ]->m_type ) << " uniform_" << i << ";\n";

					/* Reset state for next shader pass */
					m_uniforms[ i ]->m_used = false;
				}
			}

			if( ss.rdbuf()->in_avail() == 0 )
			{
				const std::string what = "\ncbuffer PixelUniforms\n{\n" + ss.str() + "};\n";

				m_source_code.insert( uniforms_offset, what );

				uniforms_offset += what.size();
			}
		}
	}

	void IGenerator::GenerateGLSL( void )
	{
		m_language = ShaderLanguage::GLSL;
		m_source_code.append( "#if defined( VERTEX )\n" );

		const size_t vertex_uniforms_offset = m_source_code.size();

		m_source_code.append( "\n" );
		for( auto it : m_attribute_layout )
		{
			std::ostringstream ss;
			ss << "ORB_ATTRIBUTE( " << it.index << " ) ";

			switch( it.GetDataCount() )
			{
				default: { assert( false ); } break;
				case 1:  { ss << "float ";  } break;
				case 2:  { ss << "vec2 ";   } break;
				case 3:  { ss << "vec3 ";   } break;
				case 4:  { ss << "vec4 ";   } break;
			}

			ss << "attribute_" << it.index << ";\n";

			m_source_code.append( ss.str() );
		}

		m_source_code.append( "\n" );
		for( auto it : m_varying_layout )
		{
			std::ostringstream ss;
			ss << "ORB_VARYING ";

			switch( it.GetDataCount() )
			{
				default: { assert( false ); } break;
				case 1:  { ss << "float ";  } break;
				case 2:  { ss << "vec2 ";   } break;
				case 3:  { ss << "vec3 ";   } break;
				case 4:  { ss << "vec4 ";   } break;
			}

			ss << "varying_" << it.index << ";\n";

			m_source_code.append( ss.str() );
		}

		m_source_code.append( "\nvoid main()\n{\n" );
		m_shader_type = ShaderType::Vertex;
		auto vs_result = VSMain();
		m_source_code.append( "\tgl_Position = " + vs_result.GetValue() + ";\n}\n" );

		{
			std::ostringstream ss;

			for( size_t i = 0; i < m_uniforms.size(); ++i )
			{
				if( m_uniforms[ i ]->m_used )
				{
					ss << "\tORB_CONSTANT( " << VariableTypeToString( m_uniforms[ i ]->m_type ) << ", uniform_" << i << " );\n";

					/* Reset state for next shader pass */
					m_uniforms[ i ]->m_used = false;
				}
			}

			if( ss.rdbuf()->in_avail() == 0 )
			{
				const std::string what = "\nORB_CONSTANTS_BEGIN( VertexConstants )\n" + ss.str() + "ORB_CONSTANTS_END\n";

				m_source_code.insert( vertex_uniforms_offset, what );
			}
		}

		m_source_code.append( "\n#elif defined( FRAGMENT )\n" );

		const size_t pixel_uniforms_insert_offset = m_source_code.size();

		m_source_code.append( "\n" );
		for( uint32_t i = 0; i < m_sampler_count; ++i )
		{
			std::ostringstream ss;
			ss << "uniform sampler2D sampler_";
			ss << i;
			ss << ";\n";

			m_source_code.append( ss.str() );
		}

		m_source_code.append( "\n" );
		for( auto it : m_varying_layout )
		{
			std::ostringstream ss;
			ss << "ORB_VARYING ";

			switch( it.GetDataCount() )
			{
				default: { assert( false ); } break;
				case 1:  { ss << "float ";  } break;
				case 2:  { ss << "vec2 ";   } break;
				case 3:  { ss << "vec3 ";   } break;
				case 4:  { ss << "vec4 ";   } break;
			}

			ss << "varying_";
			ss << it.index;
			ss << ";\n";

			m_source_code.append( ss.str() );
		}

		m_source_code.append( "\nvoid main()\n{\n" );
		m_shader_type = ShaderType::Fragment;
		auto ps_result = PSMain();
		m_source_code.append( "\tORB_SET_OUT_COLOR( " + ps_result.GetValue() + " );\n}\n" );

		{
			std::ostringstream ss;

			for( size_t i = 0; i < m_uniforms.size(); ++i )
			{
				if( m_uniforms[ i ]->m_used )
				{
					ss << "\tORB_CONSTANT( " << VariableTypeToString( m_uniforms[ i ]->m_type ) << ", uniform_" << i << " );\n";

					/* Reset state for next shader pass */
					m_uniforms[ i ]->m_used = false;
				}
			}

			if( ss.rdbuf()->in_avail() == 0 )
			{
				const std::string what = "\nORB_CONSTANTS_BEGIN( PixelConstants )\n" + ss.str() + "ORB_CONSTANTS_END\n";

				m_source_code.insert( pixel_uniforms_insert_offset, what );
			}
		}

		m_source_code.append( "\n#endif\n" );
	}
}

ORB_NAMESPACE_END
