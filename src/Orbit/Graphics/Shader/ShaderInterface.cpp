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

#include "ShaderInterface.h"

#include "Orbit/Graphics/Context/RenderContext.h"

#include <cassert>
#include <map>
#include <sstream>

ORB_NAMESPACE_BEGIN

constexpr size_t source_code_reserve_amount = 4096;

static ShaderInterface* current_shader       = nullptr;
static ShaderType       current_shader_type;
static GraphicsAPI      current_graphics_api = GraphicsAPI::Null;

ShaderInterface::Variable* ShaderInterface::Variable::s_latest_accessed_variable = nullptr;

static std::string GenerateName( const std::string prefix )
{
	static std::map< std::string, uint32_t > variable_counter;

	std::ostringstream ss;
	ss << prefix;
	ss << '_';
	ss << ( variable_counter[ prefix ]++ );
	return ss.str();
}

static std::string TypeString( ShaderInterface::VariableType type )
{
	switch( current_graphics_api )
	{
		case GraphicsAPI::D3D11:
		{
			switch( type )
			{
				case ShaderInterface::VariableType::Float: { return "float"; }
				case ShaderInterface::VariableType::Vec2:  { return "float2"; }
				case ShaderInterface::VariableType::Vec3:  { return "float3"; }
				case ShaderInterface::VariableType::Vec4:  { return "float4"; }
				case ShaderInterface::VariableType::Mat4:  { return "matrix"; }
			}
		}

		case GraphicsAPI::OpenGL:
		{
			switch( type )
			{
				case ShaderInterface::VariableType::Float: { return "float"; }
				case ShaderInterface::VariableType::Vec2:  { return "vec2"; }
				case ShaderInterface::VariableType::Vec3:  { return "vec3"; }
				case ShaderInterface::VariableType::Vec4:  { return "vec4"; }
				case ShaderInterface::VariableType::Mat4:  { return "mat4"; }
			}
		}
	}

	return "unknown";
}

ShaderInterface::ShaderInterface( void )
{
	current_shader = this;
}

ShaderInterface::~ShaderInterface( void )
{
	if( current_shader == this )
		current_shader = nullptr;
}

std::string ShaderInterface::GetSource( void )
{
	if( m_source_code.empty() )
	{
		current_shader = this;

		switch( RenderContext::Get().GetPrivateDetails().index() )
		{

		#if( ORB_HAS_D3D11 )

			case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
			{
				current_graphics_api = GraphicsAPI::D3D11;
				GenerateSourceHLSL();

			} break;

		#endif // ORB_HAS_D3D11
		#if( ORB_HAS_OPENGL )

			case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
			{
				current_graphics_api = GraphicsAPI::OpenGL;
				GenerateSourceGLSL();

			} break;

		#endif // ORB_HAS_OPENGL

		}
	}

	return m_source_code;
}

VertexLayout ShaderInterface::GetVertexLayout( void ) const
{
	return m_attribute_layout;
}

ShaderInterface::Variable::Variable( void )
{
}

ShaderInterface::Variable::Variable( const Variable& other )
	: m_value( other.m_value )
{
	other.m_used = true;
}

ShaderInterface::Variable::Variable( Variable&& other )
	: m_value ( std::move( other.m_value ) )
	, m_type  ( other.m_type )
	, m_stored( other.m_stored )
	, m_used  ( other.m_used )
{
	other.m_type   = VariableType::Unknown;
	other.m_stored = false;
	other.m_used   = false;
}

ShaderInterface::Variable::Variable( double value )
{
	std::ostringstream ss;
	ss << std::fixed << value;
	m_value = ss.str();
}

ShaderInterface::Variable::Variable( std::string_view name, VariableType type )
	: m_value( name )
	, m_type ( type )
{
}

ShaderInterface::Variable ShaderInterface::Variable::operator*( const Variable& rhs ) const
{
	m_used     = true;
	rhs.m_used = true;

	if( current_graphics_api == GraphicsAPI::D3D11 && ( m_type == VariableType::Mat4 || rhs.m_type == VariableType::Mat4 ) )
	{
		return Variable( "mul( " + rhs.GetValue() + ", " + GetValue() + " )", rhs.m_type );
	}

	return Variable( "( " + GetValue() + " * " + rhs.GetValue() + " )", m_type );
}

ShaderInterface::Variable ShaderInterface::Variable::operator+( const Variable& rhs ) const
{
	m_used     = true;
	rhs.m_used = true;

	return Variable( "( " + GetValue() + " + " + rhs.GetValue() + " )", m_type );
}

ShaderInterface::Variable ShaderInterface::Variable::operator-( void ) const
{
	m_used = true;

	return Variable( "( -" + GetValue() + " )", m_type );
}

ShaderInterface::Variable* ShaderInterface::Variable::operator->( void )
{
	StoreValue();

	m_used                     = true;
	s_latest_accessed_variable = this;

	return this;
}

void ShaderInterface::Variable::operator=( const Variable& rhs )
{
	rhs.m_used = true;

	StoreValue();
	current_shader->m_source_code.append( "\t" + GetValue() + " = " + rhs.GetValue() + ";\n" );
}

void ShaderInterface::Variable::operator+=( const Variable& rhs )
{
	rhs.m_used = true;

	StoreValue();
	current_shader->m_source_code.append( "\t" + GetValue() + " += " + rhs.GetValue() + ";\n" );
}

void ShaderInterface::Variable::operator*=( const Variable& rhs )
{
	rhs.m_used = true;

	/* TODO: if m_type == VariableType::Mat4, do mul for HLSL */

	StoreValue();
	current_shader->m_source_code.append( "\t" + GetValue() + " *= " + rhs.GetValue() + ";\n" );
}

void ShaderInterface::Variable::StoreValue( void )
{
	if( !m_stored )
	{
		auto value = m_value;
		m_value = GenerateName( "local" );

		auto typestring = TypeString( m_type );
		current_shader->m_source_code.append( "\t" + typestring + " " + m_value + " = " + value + ";\n" );

		m_stored = true;
	}
}

ShaderInterface::Float::Float( const Variable& value )
	: Variable( value.m_value )
{
	m_type       = VariableType::Float;
	value.m_used = true;
}

ShaderInterface::Vec2::Vec2( const Variable& value )
	: Variable( TypeString( VariableType::Vec2 ) + "( " + value.GetValue() + " )" )
{
	m_type       = VariableType::Vec2;
	value.m_used = true;
}

ShaderInterface::Vec2::Vec2( const Variable& value1, const Variable& value2 )
	: Variable( TypeString( VariableType::Vec2 ) + "( " + value1.GetValue() + ", " + value2.GetValue() + " )" )
{
	m_type        = VariableType::Vec2;
	value1.m_used = true;
	value2.m_used = true;
}

ShaderInterface::Vec3::Vec3( const Variable& value )
	: Variable( TypeString( VariableType::Vec3 ) + "( " + value.GetValue() + " )" )
{
	m_type       = VariableType::Vec3;
	value.m_used = true;
}

ShaderInterface::Vec3::Vec3( const Variable& value1, const Variable& value2 )
	: Variable( TypeString( VariableType::Vec3 ) + "( " + value1.GetValue() + ", " + value2.GetValue() + " )" )
{
	m_type        = VariableType::Vec3;
	value1.m_used = true;
	value2.m_used = true;
}

ShaderInterface::Vec3::Vec3( const Variable& value1, const Variable& value2, const Variable& value3 )
	: Variable( TypeString( VariableType::Vec3 ) + "( " + value1.GetValue() + ", " + value2.GetValue() + ", " + value3.GetValue() + " )" )
{
	m_type        = VariableType::Vec3;
	value1.m_used = true;
	value2.m_used = true;
	value3.m_used = true;
}

ShaderInterface::Vec4::Vec4( const Variable& value )
	: Variable( TypeString( VariableType::Vec4 ) + "( " + value.GetValue() + " )" )
{
	m_type       = VariableType::Vec4;
	value.m_used = true;
}

ShaderInterface::Vec4::Vec4( const Variable& value1, const Variable& value2 )
	: Variable( TypeString( VariableType::Vec4 ) + "( " + value1.GetValue() + ", " + value2.GetValue() + " )" )
{
	m_type        = VariableType::Vec4;
	value1.m_used = true;
	value2.m_used = true;
}

ShaderInterface::Vec4::Vec4( const Variable& value1, const Variable& value2, const Variable& value3 )
	: Variable( TypeString( VariableType::Vec4 ) + "( " + value1.GetValue() + ", " + value2.GetValue() + ", " + value3.GetValue() + " )" )
{
	m_type        = VariableType::Vec4;
	value1.m_used = true;
	value2.m_used = true;
	value3.m_used = true;
}

ShaderInterface::Vec4::Vec4( const Variable& value1, const Variable& value2, const Variable& value3, const Variable& value4 )
	: Variable( TypeString( VariableType::Vec4 ) + "( " + value1.GetValue() + ", " + value2.GetValue() + ", " + value3.GetValue() + ", " + value4.GetValue() + " )" )
{
	m_type        = VariableType::Vec4;
	value1.m_used = true;
	value2.m_used = true;
	value3.m_used = true;
	value4.m_used = true;
}

ShaderInterface::Mat4::Mat4( const Variable& value )
	: Variable( TypeString( VariableType::Mat4 ) + "( " + value.GetValue() + " )" )
{
	m_type       = VariableType::Mat4;
	value.m_used = true;
}

ShaderInterface::Sampler::Sampler( void )
	: Variable( GenerateName( "sampler" ) )
{
	m_stored = true;

	++current_shader->m_sampler_count;
}

ShaderInterface::Varying::Varying( VertexComponent component )
	: Variable( GenerateName( "varying" ) )
{
	m_stored = true;

	switch( ( IndexedVertexComponent{ component, 0 } ).GetDataCount() )
	{
		default: { assert( false );              } break;
		case 1:  { m_type = VariableType::Float; } break;
		case 2:  { m_type = VariableType::Vec2;  } break;
		case 3:  { m_type = VariableType::Vec3;  } break;
		case 4:  { m_type = VariableType::Vec4;  } break;
	}

	current_shader->m_varying_layout.Add( component );
}

std::string ShaderInterface::Varying::GetValue( void ) const
{
	if( current_graphics_api == GraphicsAPI::D3D11 )
	{
		switch( current_shader_type )
		{
			default:                   { assert( false );            } break;
			case ShaderType::Fragment: { return "input."  + m_value; }
			case ShaderType::Vertex:   { return "output." + m_value; }
		}
	}

	return m_value;
}

ShaderInterface::Attribute::Attribute( VertexComponent component )
	: Variable( GenerateName( "attribute" ) )
{
	m_stored = true;

	switch( ( IndexedVertexComponent{ component, 0 } ).GetDataCount() )
	{
		default: { assert( false );              } break;
		case 1:  { m_type = VariableType::Float; } break;
		case 2:  { m_type = VariableType::Vec2;  } break;
		case 3:  { m_type = VariableType::Vec3;  } break;
		case 4:  { m_type = VariableType::Vec4;  } break;
	}

	current_shader->m_attribute_layout.Add( component );
}

std::string ShaderInterface::Attribute::GetValue() const
{
	if( current_graphics_api == GraphicsAPI::D3D11 )
	{
		switch( current_shader_type )
		{
			default:                   { assert( false );           } break;
			case ShaderType::Fragment: { assert( false );           } break;
			case ShaderType::Vertex:   { return "input." + m_value; }
		}
	}

	return m_value;
}

ShaderInterface::UniformBase::UniformBase( VariableType type )
	: Variable( GenerateName( "uniform" ) )
{
	m_stored = true;
	m_type   = type;

	current_shader->m_uniforms.push_back( this );
}

ShaderInterface::Variable ShaderInterface::Transpose( const Variable& rhs )
{
	rhs.m_used = true;

	return Variable( "transpose( " + rhs.GetValue() + " )", VariableType::Mat4 );
}

ShaderInterface::Variable ShaderInterface::Sample( const Variable& sampler, const Variable& texcoord )
{
	sampler.m_used  = true;
	texcoord.m_used = true;

	switch( current_graphics_api )
	{
		case GraphicsAPI::D3D11:
		{
			return Variable( sampler.GetValue() + ".Sample( default_sampler_state, " + texcoord.GetValue() + " )", VariableType::Vec4 );
		}

		case GraphicsAPI::OpenGL:
		{
			return Variable( "texture( " + sampler.GetValue() + ", " + texcoord.GetValue() + " )", VariableType::Vec4 );
		}
	}

	return Variable{ };
}

ShaderInterface::Variable ShaderInterface::Dot( const Variable& vec1, const Variable& vec2 )
{
	vec1.m_used = true;
	vec2.m_used = true;

	return Variable( "dot( " + vec1.GetValue() + ", " + vec2.GetValue() + " )", VariableType::Float );
}

void ShaderInterface::GenerateSourceHLSL( void )
{
	m_source_code.reserve( source_code_reserve_amount );

	auto get_vertex_component_type_string = []( size_t data_count ) -> std::string_view
	{
		switch( data_count )
		{
			case 1: { return "float";  }
			case 2: { return "float2"; }
			case 3: { return "float3"; }
			case 4: { return "float4"; }
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
	current_shader_type = ShaderType::Vertex;
	auto vs_result = VSMain();
	m_source_code.append( "\treturn output;\n}\n" );

	{
		std::ostringstream ss;

		for( size_t i = 0; i < m_uniforms.size(); ++i )
		{
			if( m_uniforms[ i ]->m_used )
			{
				ss << "\t" << TypeString( m_uniforms[ i ]->m_type ) << " uniform_" << i << ";\n";

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
	current_shader_type = ShaderType::Fragment;
	auto ps_result = PSMain();
	m_source_code.append( "\treturn " + ps_result.GetValue() + ";\n}\n" );

	{
		std::ostringstream ss;

		for( size_t i = 0; i < m_uniforms.size(); ++i )
		{
			if( m_uniforms[ i ]->m_used )
			{
				ss << "\t" << TypeString( m_uniforms[ i ]->m_type ) << " uniform_" << i << ";\n";

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

void ShaderInterface::GenerateSourceGLSL( void )
{
	m_source_code.reserve( source_code_reserve_amount );

	m_source_code.append( "#if defined( VERTEX )\n" );

	const size_t vertex_uniforms_offset = m_source_code.size();

	m_source_code.append( "\n" );
	for( auto it : m_attribute_layout )
	{
		std::ostringstream ss;
		ss << "ORB_ATTRIBUTE( ";
		ss << it.index;
		ss << " ) ";

		switch( it.GetDataCount() )
		{
			default: { assert( false ); } break;
			case 1:  { ss << "float ";  } break;
			case 2:  { ss << "vec2 ";   } break;
			case 3:  { ss << "vec3 ";   } break;
			case 4:  { ss << "vec4 ";   } break;
		}

		ss << "attribute_";
		ss << it.index;
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
	current_shader_type = ShaderType::Vertex;
	auto vs_result = VSMain();
	m_source_code.append( "\tgl_Position = " + vs_result.GetValue() + ";\n}\n" );

	{
		std::ostringstream ss;

		for( size_t i = 0; i < m_uniforms.size(); ++i )
		{
			if( m_uniforms[ i ]->m_used )
			{
				ss << "\tORB_CONSTANT( ";
				ss << TypeString( m_uniforms[ i ]->m_type );
				ss << ", uniform_";
				ss << i;
				ss << " );\n";

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
	current_shader_type = ShaderType::Fragment;
	auto ps_result = PSMain();
	m_source_code.append( "\tORB_SET_OUT_COLOR( " + ps_result.GetValue() + " );\n}\n" );

	{
		std::ostringstream ss;

		for( size_t i = 0; i < m_uniforms.size(); ++i )
		{
			if( m_uniforms[ i ]->m_used )
			{
				ss << "\tORB_CONSTANT( ";
				ss << TypeString( m_uniforms[ i ]->m_type );
				ss << ", uniform_";
				ss << i;
				ss << " );\n";

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

ORB_NAMESPACE_END
