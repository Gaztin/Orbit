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

#include "Orbit/Core/IO/Log.h"
#include "Orbit/Graphics/Context/RenderContext.h"
#include "Orbit/ShaderGen/Generator/MainFunction.h"
#include "Orbit/ShaderGen/Variables/Uniform.h"
#include "Orbit/ShaderGen/Variables/Vec2.h"
#include "Orbit/ShaderGen/Variables/Vec4.h"

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
		switch( component.GetDataType() )
		{
			case PrimitiveDataType::Float:
			{
				switch( component.GetDataCount() )
				{
					case 1:  return "float";
					case 2:  return "vec2";
					case 3:  return "vec3";
					case 4:  return "vec4";
				}
			} break;

			case PrimitiveDataType::Int:
			{
				switch( component.GetDataCount() )
				{
					case 1:  return "int";
					case 2:  return "ivec2";
					case 3:  return "ivec3";
					case 4:  return "ivec4";
				}
			} break;
		}

		return "error_type";
	}

	static std::string_view VertexComponentSemanticName( VertexComponent component, ShaderType shader_type )
	{
		switch( component )
		{
			case VertexComponent::Position: return ( ( shader_type == ShaderType::Fragment ) ? "SV_POSITION" : "POSITION" );
			case VertexComponent::Normal:   return "NORMAL";
			case VertexComponent::Color:    return "COLOR";
			case VertexComponent::TexCoord: return "TEXCOORD";
			case VertexComponent::JointIDs: return "JOINTIDS";
			case VertexComponent::Weights:  return "WEIGHTS";
			default:                        return "ERROR";
		}
	};

#if !defined( NDEBUG )

	static void LogSourceCodeLine( const char* begin, int32_t length, int32_t line )
	{
		if( length > 0 ) LogDebug( "%3d| %.*s", line, length, begin );
		else             LogDebug( "%3d|", line );
	}

	static void LogSourceCode( std::string_view code )
	{
		int32_t line       = 0;
		int32_t line_begin = 0;

		LogDebugString( "----------------------------------------" );

		for( int32_t it = 0; it < static_cast< int32_t >( code.size() ); ++it )
		{
			if( code[ it ] == '\n' )
			{
				LogSourceCodeLine( &code[ line_begin ], ( it - line_begin ), ( ++line ) );

				line_begin = ( it + 1 );
			}
		}

		if( line_begin < static_cast< int32_t >( code.size() ) )
			LogSourceCodeLine( &code[ line_begin ], static_cast< int32_t >( line_begin - code.size() ), ( ++line ) );

		LogDebugString( "----------------------------------------" );
	}

#endif // !NDEBUG

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
		switch( RenderContext::GetInstance().GetPrivateDetails().index() )
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
		return attribute_layout_;
	}

	Variables::IVariable IGenerator::CanonicalScreenPos( const Variables::IVariable& pos )
	{
		assert( pos.data_type_ == DataType::FVec2 );

		switch( GetCurrentMainFunction()->shader_language )
		{
			case ShaderLanguage::HLSL: return Variables::Vec2( pos->x, -pos->y );
			case ShaderLanguage::GLSL: return Variables::Vec2( pos->x,  pos->y );
			default:                   return pos;
		}
	}

	Variables::IVariable IGenerator::Transpose( const Variables::IVariable& matrix )
	{
		assert( matrix.GetDataType() == DataType::Mat4 );

		matrix.SetUsed();

		return Variables::IVariable( "transpose( " + matrix.GetValue() + " )", DataType::Mat4 );
	}

	Variables::IVariable IGenerator::Sample( const Variables::IVariable& sampler, const Variables::IVariable& texcoord )
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
				Variables::IVariable var( sampler.GetValue() + ".Sample( default_sampler_state, " + texcoord.GetValue() + " )", DataType::FVec4 );
				var.StoreValue();
				return var;
			}

			case ShaderLanguage::GLSL:
			{
				Variables::IVariable var( "texture( " + sampler.GetValue() + ", " + texcoord.GetValue() + " )", DataType::FVec4 );
				var.StoreValue();
				return var;
			}
		}
	}

	Variables::IVariable IGenerator::Dot( const Variables::IVariable& lhs, const Variables::IVariable& rhs )
	{
		lhs.SetUsed();
		rhs.SetUsed();

		return Variables::IVariable( "dot( " + lhs.GetValue() + ", " + rhs.GetValue() + " )", DataType::Float );
	}

	Variables::IVariable IGenerator::Normalize( const Variables::IVariable& vec )
	{
		vec.SetUsed();

		return Variables::IVariable( "normalize( " + vec.GetValue() + " )", vec.GetDataType() );
	}

	Variables::IVariable IGenerator::Cos( const Variables::IVariable& radians )
	{
		assert( radians.data_type_ == DataType::Float );

		radians.SetUsed();

		return Variables::IVariable( "cos( " + radians.GetValue() + " )", DataType::Float );
	}

	Variables::IVariable IGenerator::Sin( const Variables::IVariable& radians )
	{
		assert( radians.data_type_ == DataType::Float );

		radians.SetUsed();

		return Variables::IVariable( "sin( " + radians.GetValue() + " )", DataType::Float );
	}

	std::string IGenerator::GenerateHLSL( void )
	{
		std::string full_source_code;

		// GLSL type names are canonical
		full_source_code.append( "#define vec2 float2\n" );
		full_source_code.append( "#define vec3 float3\n" );
		full_source_code.append( "#define vec4 float4\n" );
		full_source_code.append( "#define mat4 matrix\n" );
		full_source_code.append( "#define ivec2 int2\n" );
		full_source_code.append( "#define ivec3 int3\n" );
		full_source_code.append( "#define ivec4 int4\n" );

		if( sampler_count_ > 0 )
		{
			std::ostringstream ss;
			ss << "\n";

			for( size_t i = 0; i < sampler_count_; ++i )
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
		for( auto it : attribute_layout_ )
		{
			auto type_string   = VertexComponentTypeString( it );
			auto semantic_name = VertexComponentSemanticName( it.type, ShaderType::Vertex );

			std::ostringstream ss;
			ss << "\t" << type_string << " attribute_" << it.index << " : " << semantic_name << ";\n";

			full_source_code.append( ss.str() );
		}
		full_source_code.append( "};\n" );

		full_source_code.append( "\nstruct PixelData\n{\n" );
		for( auto it : varying_layout_ )
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

			for( size_t i = 0; i < uniforms_.size(); ++i )
			{
				if( uniforms_[ i ]->used_ )
				{
					if( uniforms_[ i ]->IsArray() )
					{
						const Variables::UniformArrayBase* uniform_array = static_cast< const Variables::UniformArrayBase* >( uniforms_[ i ] );

						ss << "\t" << DataTypeToString( uniform_array->GetElementType() ) << " uniform_" << i << "[ " << uniform_array->GetArraySize() << " ];\n";
					}
					else
					{
						ss << "\t" << DataTypeToString( uniforms_[ i ]->data_type_ ) << " uniform_" << i << ";\n";
					}


					/* Reset state for next shader pass */
					uniforms_[ i ]->used_ = false;
				}
			}

			const std::string ss_str = ss.str();

			if( !ss_str.empty() )
			{
				const std::string what = "\ncbuffer VertexUniforms\n{\n" + ss_str + "};\n";

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

			for( size_t i = 0; i < uniforms_.size(); ++i )
			{
				if( uniforms_[ i ]->used_ )
				{
					ss << "\t" << DataTypeToString( uniforms_[ i ]->data_type_ ) << " uniform_" << i << ";\n";

					/* Reset state for next shader pass */
					uniforms_[ i ]->used_ = false;
				}
			}

			const std::string ss_str = ss.str();

			if( !ss_str.empty() )
			{
				const std::string what = "\ncbuffer PixelUniforms\n{\n" + ss_str + "};\n";

				full_source_code.insert( uniforms_offset, what );

				uniforms_offset += what.size();
			}
		}

	#if defined( _DEBUG )
		LogSourceCode( full_source_code );
	#endif // _DEBUG

		return full_source_code;
	}

	std::string IGenerator::GenerateGLSL( void )
	{
		std::string full_source_code;

		full_source_code.append( "#if defined( VERTEX )\n" );

		const size_t vertex_uniforms_offset = full_source_code.size();

		full_source_code.append( "\n" );
		for( auto it : attribute_layout_ )
		{
			std::ostringstream ss;
			ss << "ORB_ATTRIBUTE( " << it.index << " ) " << VertexComponentTypeString( it ) << " attribute_" << it.index << ";\n";

			full_source_code.append( ss.str() );
		}

		full_source_code.append( "\n" );
		for( auto it : varying_layout_ )
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

			for( size_t i = 0; i < uniforms_.size(); ++i )
			{
				if( uniforms_[ i ]->used_ )
				{
					if( uniforms_[ i ]->IsArray() )
					{
						const Variables::UniformArrayBase* uniform_array = static_cast< const Variables::UniformArrayBase* >( uniforms_[ i ] );

						ss << "\tORB_CONSTANT( " << DataTypeToString( uniform_array->GetElementType() ) << ", uniform_" << i << "[ " << uniform_array->GetArraySize() << " ] );\n";
					}
					else
					{
						ss << "\tORB_CONSTANT( " << DataTypeToString( uniforms_[ i ]->data_type_ ) << ", uniform_" << i << " );\n";
					}

					/* Reset state for next shader pass */
					uniforms_[ i ]->used_ = false;
				}
			}

			const std::string ss_str = ss.str();

			if( !ss_str.empty() )
			{
				const std::string what = "\nORB_CONSTANTS_BEGIN( VertexConstants )\n" + ss_str + "ORB_CONSTANTS_END\n";

				full_source_code.insert( vertex_uniforms_offset, what );
			}
		}

		full_source_code.append( "\n#elif defined( FRAGMENT )\n" );

		const size_t pixel_uniforms_insert_offset = full_source_code.size();

		full_source_code.append( "\n" );
		for( uint32_t i = 0; i < sampler_count_; ++i )
		{
			std::ostringstream ss;
			ss << "uniform sampler2D sampler_";
			ss << i;
			ss << ";\n";

			full_source_code.append( ss.str() );
		}

		full_source_code.append( "\n" );
		for( auto it : varying_layout_ )
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

			for( size_t i = 0; i < uniforms_.size(); ++i )
			{
				if( uniforms_[ i ]->used_ )
				{
					ss << "\tORB_CONSTANT( " << DataTypeToString( uniforms_[ i ]->data_type_ ) << ", uniform_" << i << " );\n";

					/* Reset state for next shader pass */
					uniforms_[ i ]->used_ = false;
				}
			}

			const std::string ss_str = ss.str();

			if( !ss_str.empty() )
			{
				const std::string what = "\nORB_CONSTANTS_BEGIN( PixelConstants )\n" + ss_str + "ORB_CONSTANTS_END\n";

				full_source_code.insert( pixel_uniforms_insert_offset, what );
			}
		}

		full_source_code.append( "\n#endif\n" );

	#if defined( _DEBUG )
		LogSourceCode( full_source_code );
	#endif // _DEBUG

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
