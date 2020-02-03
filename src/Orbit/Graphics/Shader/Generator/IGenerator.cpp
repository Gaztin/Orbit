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
#include "Orbit/Graphics/Shader/Generator/Variables/Vec4.h"
#include "Orbit/Graphics/Shader/Generator/MainFunction.h"

#include <cassert>
#include <map>
#include <sstream>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	static IGenerator*   current_generator     = nullptr;
	static MainFunction* current_main_function = nullptr;

	static std::string_view VertexComponentTypeString( IndexedVertexComponent component )
	{
		switch( component.GetDataCount() )
		{
			case 1:  return "float";
			case 2:  return "vec2";
			case 3:  return "vec3";
			case 4:  return "vec4";
			default: return "error_type";
		}
	}

	static std::string_view VertexComponentSemanticName( VertexComponent component, ShaderType shader_type )
	{
		switch( component )
		{
			case VertexComponent::Position: return ( ( shader_type == ShaderType::Fragment ) ? "SV_POSITION" : "POSITION" );
			case VertexComponent::Normal:   return "NORMAL";
			case VertexComponent::Color:    return "COLOR";
			case VertexComponent::TexCoord: return "TEXCOORD";
			default:                        return "ERROR";
		}
	};

	IGenerator::IGenerator( void )
	{
		current_generator = this;
	}

	IGenerator::~IGenerator( void )
	{
		if( current_generator == this )
			current_generator = nullptr;
	}

	std::string IGenerator::Generate( void )
	{
		switch( RenderContext::Get().GetPrivateDetails().index() )
		{
			default:
			{
				return std::string();
			}

		#if( ORB_HAS_D3D11 )

			case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
			{
				return GenerateHLSL();
			};

		#endif // ORB_HAS_D3D11
		#if( ORB_HAS_OPENGL )

			case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
			{
				return GenerateGLSL();
			}

		#endif // ORB_HAS_OPENGL

		}
	}

	VertexLayout IGenerator::GetVertexLayout( void ) const
	{
		return m_attribute_layout;
	}

	IVariable IGenerator::Transpose( const IVariable& matrix )
	{
		assert( matrix.GetDataType() == DataType::Mat4 );

		matrix.SetUsed();

		return IVariable( "transpose( " + matrix.GetValue() + " )", DataType::Mat4 );
	}

	IVariable IGenerator::Sample( const IVariable& sampler, const IVariable& texcoord )
	{
		sampler.SetUsed();
		texcoord.SetUsed();

		switch( GetCurrentMainFunction()->shader_language )
		{
			default:
			{
				assert( false );
				return { };
			}

			case ShaderLanguage::HLSL:
			{
				return IVariable( sampler.GetValue() + ".Sample( default_sampler_state, " + texcoord.GetValue() + " )", DataType::Vec4 );
			}

			case ShaderLanguage::GLSL:
			{
				return IVariable( "texture( " + sampler.GetValue() + ", " + texcoord.GetValue() + " )", DataType::Vec4 );
			}
		}
	}

	IVariable IGenerator::Dot( const IVariable& lhs, const IVariable& rhs )
	{
		lhs.SetUsed();
		rhs.SetUsed();

		return IVariable( "dot( " + lhs.GetValue() + ", " + rhs.GetValue() + " )", DataType::Float );
	}

	IVariable IGenerator::Normalize( const IVariable& vec )
	{
		vec.SetUsed();

		return IVariable( "normalize( " + vec.GetValue() + " )", vec.GetDataType() );
	}

	std::string IGenerator::GenerateHLSL( void )
	{
		std::string full_source_code;

		// GLSL type names are canonical
		full_source_code.append( "#define vec2 float2\n" );
		full_source_code.append( "#define vec3 float3\n" );
		full_source_code.append( "#define vec4 float4\n" );
		full_source_code.append( "#define mat4 matrix\n" );

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

			full_source_code.append( ss.str() );
		}

		size_t uniforms_offset = full_source_code.size();

		full_source_code.append( "\nstruct VertexData\n{\n" );
		for( auto it : m_attribute_layout )
		{
			auto type_string   = VertexComponentTypeString( it );
			auto semantic_name = VertexComponentSemanticName( it.type, ShaderType::Vertex );

			std::ostringstream ss;
			ss << "\t" << type_string << " attribute_" << it.index << " : " << semantic_name << ";\n";

			full_source_code.append( ss.str() );
		}
		full_source_code.append( "};\n" );

		full_source_code.append( "\nstruct PixelData\n{\n" );
		for( auto it : m_varying_layout )
		{
			auto type_string   = VertexComponentTypeString( it );
			auto semantic_name = VertexComponentSemanticName( it.type, ShaderType::Fragment );

			std::ostringstream ss;
			ss << "\t" << type_string << " varying_" << it.index << " : " << semantic_name << ";\n";

			full_source_code.append( ss.str() );
		}
		full_source_code.append( "};\n" );

		/* Generate main function for the vertex shader */
		{
			MainFunction vs_main;
			vs_main.shader_language = ShaderLanguage::HLSL;
			vs_main.shader_type     = ShaderType::Vertex;

			current_main_function = &vs_main;

			full_source_code.append( "\nPixelData VSMain( VertexData input )\n{\n\tPixelData output;\n" );

			Vec4 vs_result = VSMain();

			full_source_code.append( vs_main.code.str() );
			full_source_code.append( "\treturn output;\n}\n" );

			current_main_function = nullptr;
		}

		{
			std::ostringstream ss;

			for( size_t i = 0; i < m_uniforms.size(); ++i )
			{
				if( m_uniforms[ i ]->m_used )
				{
					ss << "\t" << DataTypeToString( m_uniforms[ i ]->m_data_type ) << " uniform_" << i << ";\n";

					/* Reset state for next shader pass */
					m_uniforms[ i ]->m_used = false;
				}
			}

			if( ss.rdbuf()->in_avail() == 0 )
			{
				const std::string what = "\ncbuffer VertexUniforms\n{\n" + ss.str() + "};\n";

				full_source_code.insert( uniforms_offset, what );

				uniforms_offset += what.size();
			}
		}

		/* Generate main function for the pixel shader */
		{
			MainFunction ps_main;
			ps_main.shader_language = ShaderLanguage::HLSL;
			ps_main.shader_type     = ShaderType::Fragment;

			current_main_function = &ps_main;

			full_source_code.append( "\nfloat4 PSMain( PixelData input ) : SV_TARGET\n{\n" );

			Vec4 ps_result = PSMain();

			full_source_code.append( ps_main.code.str() );
			full_source_code.append( "\treturn " + ps_result.GetValue() + ";\n}\n" );

			current_main_function = nullptr;
		}

		{
			std::ostringstream ss;

			for( size_t i = 0; i < m_uniforms.size(); ++i )
			{
				if( m_uniforms[ i ]->m_used )
				{
					ss << "\t" << DataTypeToString( m_uniforms[ i ]->m_data_type ) << " uniform_" << i << ";\n";

					/* Reset state for next shader pass */
					m_uniforms[ i ]->m_used = false;
				}
			}

			if( ss.rdbuf()->in_avail() == 0 )
			{
				const std::string what = "\ncbuffer PixelUniforms\n{\n" + ss.str() + "};\n";

				full_source_code.insert( uniforms_offset, what );

				uniforms_offset += what.size();
			}
		}

		return full_source_code;
	}

	std::string IGenerator::GenerateGLSL( void )
	{
		std::string full_source_code;

		full_source_code.append( "#if defined( VERTEX )\n" );

		const size_t vertex_uniforms_offset = full_source_code.size();

		full_source_code.append( "\n" );
		for( auto it : m_attribute_layout )
		{
			std::ostringstream ss;
			ss << "ORB_ATTRIBUTE( " << it.index << " ) " << VertexComponentTypeString( it ) << " attribute_" << it.index << ";\n";

			full_source_code.append( ss.str() );
		}

		full_source_code.append( "\n" );
		for( auto it : m_varying_layout )
		{
			std::ostringstream ss;
			ss << "ORB_VARYING " << VertexComponentTypeString( it ) << " varying_" << it.index << ";\n";

			full_source_code.append( ss.str() );
		}

		/* Generate main function for vertex shader */
		{
			MainFunction vs_main;
			vs_main.shader_language = ShaderLanguage::GLSL;
			vs_main.shader_type     = ShaderType::Vertex;

			current_main_function = &vs_main;

			full_source_code.append( "\nvoid main()\n{\n" );

			Vec4 vs_result = VSMain();

			full_source_code.append( vs_main.code.str() );
			full_source_code.append( "\tgl_Position = " + vs_result.GetValue() + ";\n}\n" );

			current_main_function = nullptr;
		}

		{
			std::ostringstream ss;

			for( size_t i = 0; i < m_uniforms.size(); ++i )
			{
				if( m_uniforms[ i ]->m_used )
				{
					ss << "\tORB_CONSTANT( " << DataTypeToString( m_uniforms[ i ]->m_data_type ) << ", uniform_" << i << " );\n";

					/* Reset state for next shader pass */
					m_uniforms[ i ]->m_used = false;
				}
			}

			if( ss.rdbuf()->in_avail() == 0 )
			{
				const std::string what = "\nORB_CONSTANTS_BEGIN( VertexConstants )\n" + ss.str() + "ORB_CONSTANTS_END\n";

				full_source_code.insert( vertex_uniforms_offset, what );
			}
		}

		full_source_code.append( "\n#elif defined( FRAGMENT )\n" );

		const size_t pixel_uniforms_insert_offset = full_source_code.size();

		full_source_code.append( "\n" );
		for( uint32_t i = 0; i < m_sampler_count; ++i )
		{
			std::ostringstream ss;
			ss << "uniform sampler2D sampler_";
			ss << i;
			ss << ";\n";

			full_source_code.append( ss.str() );
		}

		full_source_code.append( "\n" );
		for( auto it : m_varying_layout )
		{
			std::ostringstream ss;
			ss << "ORB_VARYING " << VertexComponentTypeString( it ) << " varying_" << it.index << ";\n";

			full_source_code.append( ss.str() );
		}

		/* Generate main function for the fragment shader */
		{
			MainFunction ps_main;
			ps_main.shader_language = ShaderLanguage::GLSL;
			ps_main.shader_type     = ShaderType::Fragment;

			current_main_function = &ps_main;

			full_source_code.append( "\nvoid main()\n{\n" );

			Vec4 ps_result = PSMain();

			full_source_code.append( ps_main.code.str() );
			full_source_code.append( "\tORB_SET_OUT_COLOR( " + ps_result.GetValue() + " );\n}\n" );

			current_main_function = nullptr;
		}

		{
			std::ostringstream ss;

			for( size_t i = 0; i < m_uniforms.size(); ++i )
			{
				if( m_uniforms[ i ]->m_used )
				{
					ss << "\tORB_CONSTANT( " << DataTypeToString( m_uniforms[ i ]->m_data_type ) << ", uniform_" << i << " );\n";

					/* Reset state for next shader pass */
					m_uniforms[ i ]->m_used = false;
				}
			}

			if( ss.rdbuf()->in_avail() == 0 )
			{
				const std::string what = "\nORB_CONSTANTS_BEGIN( PixelConstants )\n" + ss.str() + "ORB_CONSTANTS_END\n";

				full_source_code.insert( pixel_uniforms_insert_offset, what );
			}
		}

		full_source_code.append( "\n#endif\n" );

		return full_source_code;
	}

	IGenerator* IGenerator::GetCurrentGenerator( void )
	{
		return current_generator;
	}

	MainFunction* IGenerator::GetCurrentMainFunction( void )
	{
		return current_main_function;
	}
}

ORB_NAMESPACE_END
