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

#include "Orbit/Core/IO/Log.h"
#include "Orbit/Graphics/Context/RenderContext.h"

#include <cassert>
#include <map>
#include <sstream>

ORB_NAMESPACE_BEGIN

constexpr size_t source_code_reserve_amount = 4096;

static ShaderInterface* current_shader       = nullptr;
static ShaderType       current_shader_type;
static GraphicsAPI      current_graphics_api = GraphicsAPI::Null;

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

	LogInfo( "%s\n", m_source_code.c_str() );

	return m_source_code;
}

VertexLayout ShaderInterface::GetVertexLayout( void ) const
{
	return m_attribute_layout;
}

void ShaderInterface::VariableDummy::operator*=( const Variable& rhs ) const
{
	rhs.m_used = true;

	parent->StoreValue();
	current_shader->m_source_code.append( "\t" + parent->GetValue() + "." + value + " *= " + rhs.GetValue() + ";\n" );
}

ShaderInterface::VariableDummy::operator ShaderInterface::Variable( void ) const
{
	return Variable( parent->GetValue() + "." + value, type );
}

ShaderInterface::Variable::Variable( void )
{
	InitDummies();
}

ShaderInterface::Variable::Variable( const Variable& other )
	: m_value( other.m_value )
{
	other.m_used = true;

	InitDummies();
}

ShaderInterface::Variable::Variable( Variable&& other )
	: m_value( std::move( other.m_value ) )
{
	other.m_used = true;
}

ShaderInterface::Variable::Variable( double value )
{
	std::ostringstream ss;
	ss << std::fixed << value;
	m_value = ss.str();

	InitDummies();
}

ShaderInterface::Variable::Variable( std::string_view name, VariableType type )
	: m_value( name )
	, m_type ( type )
{
	InitDummies();
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
	current_shader->m_source_code.append( "( " + GetValue() + " += " + rhs.GetValue() + " )" );
}

void ShaderInterface::Variable::operator*=( const Variable& rhs )
{
	rhs.m_used = true;

	/* TODO: if m_type == VariableType::Mat4, do mul for HLSL */

	StoreValue();
	current_shader->m_source_code.append( "( " + GetValue() + " *= " + rhs.GetValue() + " )" );
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

void ShaderInterface::Variable::InitDummies( void )
{
	x    = VariableDummy{ this, "x", VariableType::Float };
	y    = VariableDummy{ this, "y", VariableType::Float };
	z    = VariableDummy{ this, "z", VariableType::Float };
	w    = VariableDummy{ this, "w", VariableType::Float };

	xx   = VariableDummy{ this, "xx", VariableType::Vec2 };
	xy   = VariableDummy{ this, "xy", VariableType::Vec2 };
	xz   = VariableDummy{ this, "xz", VariableType::Vec2 };
	xw   = VariableDummy{ this, "xw", VariableType::Vec2 };
	yx   = VariableDummy{ this, "yx", VariableType::Vec2 };
	yy   = VariableDummy{ this, "yy", VariableType::Vec2 };
	yz   = VariableDummy{ this, "yz", VariableType::Vec2 };
	yw   = VariableDummy{ this, "yw", VariableType::Vec2 };
	zx   = VariableDummy{ this, "zx", VariableType::Vec2 };
	zy   = VariableDummy{ this, "zy", VariableType::Vec2 };
	zz   = VariableDummy{ this, "zz", VariableType::Vec2 };
	zw   = VariableDummy{ this, "zw", VariableType::Vec2 };
	wx   = VariableDummy{ this, "wx", VariableType::Vec2 };
	wy   = VariableDummy{ this, "wy", VariableType::Vec2 };
	wz   = VariableDummy{ this, "wz", VariableType::Vec2 };
	ww   = VariableDummy{ this, "ww", VariableType::Vec2 };

	xxx  = VariableDummy{ this, "xxx", VariableType::Vec3 };
	xxy  = VariableDummy{ this, "xxy", VariableType::Vec3 };
	xxz  = VariableDummy{ this, "xxz", VariableType::Vec3 };
	xxw  = VariableDummy{ this, "xxw", VariableType::Vec3 };
	xyx  = VariableDummy{ this, "xyx", VariableType::Vec3 };
	xyy  = VariableDummy{ this, "xyy", VariableType::Vec3 };
	xyz  = VariableDummy{ this, "xyz", VariableType::Vec3 };
	xyw  = VariableDummy{ this, "xyw", VariableType::Vec3 };
	xzx  = VariableDummy{ this, "xzx", VariableType::Vec3 };
	xzy  = VariableDummy{ this, "xzy", VariableType::Vec3 };
	xzz  = VariableDummy{ this, "xzz", VariableType::Vec3 };
	xzw  = VariableDummy{ this, "xzw", VariableType::Vec3 };
	xwx  = VariableDummy{ this, "xwx", VariableType::Vec3 };
	xwy  = VariableDummy{ this, "xwy", VariableType::Vec3 };
	xwz  = VariableDummy{ this, "xwz", VariableType::Vec3 };
	xww  = VariableDummy{ this, "xww", VariableType::Vec3 };
	yxx  = VariableDummy{ this, "yxx", VariableType::Vec3 };
	yxy  = VariableDummy{ this, "yxy", VariableType::Vec3 };
	yxz  = VariableDummy{ this, "yxz", VariableType::Vec3 };
	yxw  = VariableDummy{ this, "yxw", VariableType::Vec3 };
	yyx  = VariableDummy{ this, "yyx", VariableType::Vec3 };
	yyy  = VariableDummy{ this, "yyy", VariableType::Vec3 };
	yyz  = VariableDummy{ this, "yyz", VariableType::Vec3 };
	yyw  = VariableDummy{ this, "yyw", VariableType::Vec3 };
	yzx  = VariableDummy{ this, "yzx", VariableType::Vec3 };
	yzy  = VariableDummy{ this, "yzy", VariableType::Vec3 };
	yzz  = VariableDummy{ this, "yzz", VariableType::Vec3 };
	yzw  = VariableDummy{ this, "yzw", VariableType::Vec3 };
	ywx  = VariableDummy{ this, "ywx", VariableType::Vec3 };
	ywy  = VariableDummy{ this, "ywy", VariableType::Vec3 };
	ywz  = VariableDummy{ this, "ywz", VariableType::Vec3 };
	yww  = VariableDummy{ this, "yww", VariableType::Vec3 };
	zxx  = VariableDummy{ this, "zxx", VariableType::Vec3 };
	zxy  = VariableDummy{ this, "zxy", VariableType::Vec3 };
	zxz  = VariableDummy{ this, "zxz", VariableType::Vec3 };
	zxw  = VariableDummy{ this, "zxw", VariableType::Vec3 };
	zyx  = VariableDummy{ this, "zyx", VariableType::Vec3 };
	zyy  = VariableDummy{ this, "zyy", VariableType::Vec3 };
	zyz  = VariableDummy{ this, "zyz", VariableType::Vec3 };
	zyw  = VariableDummy{ this, "zyw", VariableType::Vec3 };
	zzx  = VariableDummy{ this, "zzx", VariableType::Vec3 };
	zzy  = VariableDummy{ this, "zzy", VariableType::Vec3 };
	zzz  = VariableDummy{ this, "zzz", VariableType::Vec3 };
	zzw  = VariableDummy{ this, "zzw", VariableType::Vec3 };
	zwx  = VariableDummy{ this, "zwx", VariableType::Vec3 };
	zwy  = VariableDummy{ this, "zwy", VariableType::Vec3 };
	zwz  = VariableDummy{ this, "zwz", VariableType::Vec3 };
	zww  = VariableDummy{ this, "zww", VariableType::Vec3 };
	wxx  = VariableDummy{ this, "wxx", VariableType::Vec3 };
	wxy  = VariableDummy{ this, "wxy", VariableType::Vec3 };
	wxz  = VariableDummy{ this, "wxz", VariableType::Vec3 };
	wxw  = VariableDummy{ this, "wxw", VariableType::Vec3 };
	wyx  = VariableDummy{ this, "wyx", VariableType::Vec3 };
	wyy  = VariableDummy{ this, "wyy", VariableType::Vec3 };
	wyz  = VariableDummy{ this, "wyz", VariableType::Vec3 };
	wyw  = VariableDummy{ this, "wyw", VariableType::Vec3 };
	wzx  = VariableDummy{ this, "wzx", VariableType::Vec3 };
	wzy  = VariableDummy{ this, "wzy", VariableType::Vec3 };
	wzz  = VariableDummy{ this, "wzz", VariableType::Vec3 };
	wzw  = VariableDummy{ this, "wzw", VariableType::Vec3 };
	wwx  = VariableDummy{ this, "wwx", VariableType::Vec3 };
	wwy  = VariableDummy{ this, "wwy", VariableType::Vec3 };
	wwz  = VariableDummy{ this, "wwz", VariableType::Vec3 };
	www  = VariableDummy{ this, "www", VariableType::Vec3 };

	xxxx = VariableDummy{ this, "xxxx", VariableType::Vec4 };
	xxxy = VariableDummy{ this, "xxxy", VariableType::Vec4 };
	xxxz = VariableDummy{ this, "xxxz", VariableType::Vec4 };
	xxxw = VariableDummy{ this, "xxxw", VariableType::Vec4 };
	xxyx = VariableDummy{ this, "xxyx", VariableType::Vec4 };
	xxyy = VariableDummy{ this, "xxyy", VariableType::Vec4 };
	xxyz = VariableDummy{ this, "xxyz", VariableType::Vec4 };
	xxyw = VariableDummy{ this, "xxyw", VariableType::Vec4 };
	xxzx = VariableDummy{ this, "xxzx", VariableType::Vec4 };
	xxzy = VariableDummy{ this, "xxzy", VariableType::Vec4 };
	xxzz = VariableDummy{ this, "xxzz", VariableType::Vec4 };
	xxzw = VariableDummy{ this, "xxzw", VariableType::Vec4 };
	xxwx = VariableDummy{ this, "xxwx", VariableType::Vec4 };
	xxwy = VariableDummy{ this, "xxwy", VariableType::Vec4 };
	xxwz = VariableDummy{ this, "xxwz", VariableType::Vec4 };
	xxww = VariableDummy{ this, "xxww", VariableType::Vec4 };
	xyxx = VariableDummy{ this, "xyxx", VariableType::Vec4 };
	xyxy = VariableDummy{ this, "xyxy", VariableType::Vec4 };
	xyxz = VariableDummy{ this, "xyxz", VariableType::Vec4 };
	xyxw = VariableDummy{ this, "xyxw", VariableType::Vec4 };
	xyyx = VariableDummy{ this, "xyyx", VariableType::Vec4 };
	xyyy = VariableDummy{ this, "xyyy", VariableType::Vec4 };
	xyyz = VariableDummy{ this, "xyyz", VariableType::Vec4 };
	xyyw = VariableDummy{ this, "xyyw", VariableType::Vec4 };
	xyzx = VariableDummy{ this, "xyzx", VariableType::Vec4 };
	xyzy = VariableDummy{ this, "xyzy", VariableType::Vec4 };
	xyzz = VariableDummy{ this, "xyzz", VariableType::Vec4 };
	xyzw = VariableDummy{ this, "xyzw", VariableType::Vec4 };
	xywx = VariableDummy{ this, "xywx", VariableType::Vec4 };
	xywy = VariableDummy{ this, "xywy", VariableType::Vec4 };
	xywz = VariableDummy{ this, "xywz", VariableType::Vec4 };
	xyww = VariableDummy{ this, "xyww", VariableType::Vec4 };
	xzxx = VariableDummy{ this, "xzxx", VariableType::Vec4 };
	xzxy = VariableDummy{ this, "xzxy", VariableType::Vec4 };
	xzxz = VariableDummy{ this, "xzxz", VariableType::Vec4 };
	xzxw = VariableDummy{ this, "xzxw", VariableType::Vec4 };
	xzyx = VariableDummy{ this, "xzyx", VariableType::Vec4 };
	xzyy = VariableDummy{ this, "xzyy", VariableType::Vec4 };
	xzyz = VariableDummy{ this, "xzyz", VariableType::Vec4 };
	xzyw = VariableDummy{ this, "xzyw", VariableType::Vec4 };
	xzzx = VariableDummy{ this, "xzzx", VariableType::Vec4 };
	xzzy = VariableDummy{ this, "xzzy", VariableType::Vec4 };
	xzzz = VariableDummy{ this, "xzzz", VariableType::Vec4 };
	xzzw = VariableDummy{ this, "xzzw", VariableType::Vec4 };
	xzwx = VariableDummy{ this, "xzwx", VariableType::Vec4 };
	xzwy = VariableDummy{ this, "xzwy", VariableType::Vec4 };
	xzwz = VariableDummy{ this, "xzwz", VariableType::Vec4 };
	xzww = VariableDummy{ this, "xzww", VariableType::Vec4 };
	xwxx = VariableDummy{ this, "xwxx", VariableType::Vec4 };
	xwxy = VariableDummy{ this, "xwxy", VariableType::Vec4 };
	xwxz = VariableDummy{ this, "xwxz", VariableType::Vec4 };
	xwxw = VariableDummy{ this, "xwxw", VariableType::Vec4 };
	xwyx = VariableDummy{ this, "xwyx", VariableType::Vec4 };
	xwyy = VariableDummy{ this, "xwyy", VariableType::Vec4 };
	xwyz = VariableDummy{ this, "xwyz", VariableType::Vec4 };
	xwyw = VariableDummy{ this, "xwyw", VariableType::Vec4 };
	xwzx = VariableDummy{ this, "xwzx", VariableType::Vec4 };
	xwzy = VariableDummy{ this, "xwzy", VariableType::Vec4 };
	xwzz = VariableDummy{ this, "xwzz", VariableType::Vec4 };
	xwzw = VariableDummy{ this, "xwzw", VariableType::Vec4 };
	xwwx = VariableDummy{ this, "xwwx", VariableType::Vec4 };
	xwwy = VariableDummy{ this, "xwwy", VariableType::Vec4 };
	xwwz = VariableDummy{ this, "xwwz", VariableType::Vec4 };
	xwww = VariableDummy{ this, "xwww", VariableType::Vec4 };
	yxxx = VariableDummy{ this, "yxxx", VariableType::Vec4 };
	yxxy = VariableDummy{ this, "yxxy", VariableType::Vec4 };
	yxxz = VariableDummy{ this, "yxxz", VariableType::Vec4 };
	yxxw = VariableDummy{ this, "yxxw", VariableType::Vec4 };
	yxyx = VariableDummy{ this, "yxyx", VariableType::Vec4 };
	yxyy = VariableDummy{ this, "yxyy", VariableType::Vec4 };
	yxyz = VariableDummy{ this, "yxyz", VariableType::Vec4 };
	yxyw = VariableDummy{ this, "yxyw", VariableType::Vec4 };
	yxzx = VariableDummy{ this, "yxzx", VariableType::Vec4 };
	yxzy = VariableDummy{ this, "yxzy", VariableType::Vec4 };
	yxzz = VariableDummy{ this, "yxzz", VariableType::Vec4 };
	yxzw = VariableDummy{ this, "yxzw", VariableType::Vec4 };
	yxwx = VariableDummy{ this, "yxwx", VariableType::Vec4 };
	yxwy = VariableDummy{ this, "yxwy", VariableType::Vec4 };
	yxwz = VariableDummy{ this, "yxwz", VariableType::Vec4 };
	yxww = VariableDummy{ this, "yxww", VariableType::Vec4 };
	yyxx = VariableDummy{ this, "yyxx", VariableType::Vec4 };
	yyxy = VariableDummy{ this, "yyxy", VariableType::Vec4 };
	yyxz = VariableDummy{ this, "yyxz", VariableType::Vec4 };
	yyxw = VariableDummy{ this, "yyxw", VariableType::Vec4 };
	yyyx = VariableDummy{ this, "yyyx", VariableType::Vec4 };
	yyyy = VariableDummy{ this, "yyyy", VariableType::Vec4 };
	yyyz = VariableDummy{ this, "yyyz", VariableType::Vec4 };
	yyyw = VariableDummy{ this, "yyyw", VariableType::Vec4 };
	yyzx = VariableDummy{ this, "yyzx", VariableType::Vec4 };
	yyzy = VariableDummy{ this, "yyzy", VariableType::Vec4 };
	yyzz = VariableDummy{ this, "yyzz", VariableType::Vec4 };
	yyzw = VariableDummy{ this, "yyzw", VariableType::Vec4 };
	yywx = VariableDummy{ this, "yywx", VariableType::Vec4 };
	yywy = VariableDummy{ this, "yywy", VariableType::Vec4 };
	yywz = VariableDummy{ this, "yywz", VariableType::Vec4 };
	yyww = VariableDummy{ this, "yyww", VariableType::Vec4 };
	yzxx = VariableDummy{ this, "yzxx", VariableType::Vec4 };
	yzxy = VariableDummy{ this, "yzxy", VariableType::Vec4 };
	yzxz = VariableDummy{ this, "yzxz", VariableType::Vec4 };
	yzxw = VariableDummy{ this, "yzxw", VariableType::Vec4 };
	yzyx = VariableDummy{ this, "yzyx", VariableType::Vec4 };
	yzyy = VariableDummy{ this, "yzyy", VariableType::Vec4 };
	yzyz = VariableDummy{ this, "yzyz", VariableType::Vec4 };
	yzyw = VariableDummy{ this, "yzyw", VariableType::Vec4 };
	yzzx = VariableDummy{ this, "yzzx", VariableType::Vec4 };
	yzzy = VariableDummy{ this, "yzzy", VariableType::Vec4 };
	yzzz = VariableDummy{ this, "yzzz", VariableType::Vec4 };
	yzzw = VariableDummy{ this, "yzzw", VariableType::Vec4 };
	yzwx = VariableDummy{ this, "yzwx", VariableType::Vec4 };
	yzwy = VariableDummy{ this, "yzwy", VariableType::Vec4 };
	yzwz = VariableDummy{ this, "yzwz", VariableType::Vec4 };
	yzww = VariableDummy{ this, "yzww", VariableType::Vec4 };
	ywxx = VariableDummy{ this, "ywxx", VariableType::Vec4 };
	ywxy = VariableDummy{ this, "ywxy", VariableType::Vec4 };
	ywxz = VariableDummy{ this, "ywxz", VariableType::Vec4 };
	ywxw = VariableDummy{ this, "ywxw", VariableType::Vec4 };
	ywyx = VariableDummy{ this, "ywyx", VariableType::Vec4 };
	ywyy = VariableDummy{ this, "ywyy", VariableType::Vec4 };
	ywyz = VariableDummy{ this, "ywyz", VariableType::Vec4 };
	ywyw = VariableDummy{ this, "ywyw", VariableType::Vec4 };
	ywzx = VariableDummy{ this, "ywzx", VariableType::Vec4 };
	ywzy = VariableDummy{ this, "ywzy", VariableType::Vec4 };
	ywzz = VariableDummy{ this, "ywzz", VariableType::Vec4 };
	ywzw = VariableDummy{ this, "ywzw", VariableType::Vec4 };
	ywwx = VariableDummy{ this, "ywwx", VariableType::Vec4 };
	ywwy = VariableDummy{ this, "ywwy", VariableType::Vec4 };
	ywwz = VariableDummy{ this, "ywwz", VariableType::Vec4 };
	ywww = VariableDummy{ this, "ywww", VariableType::Vec4 };
	zxxx = VariableDummy{ this, "zxxx", VariableType::Vec4 };
	zxxy = VariableDummy{ this, "zxxy", VariableType::Vec4 };
	zxxz = VariableDummy{ this, "zxxz", VariableType::Vec4 };
	zxxw = VariableDummy{ this, "zxxw", VariableType::Vec4 };
	zxyx = VariableDummy{ this, "zxyx", VariableType::Vec4 };
	zxyy = VariableDummy{ this, "zxyy", VariableType::Vec4 };
	zxyz = VariableDummy{ this, "zxyz", VariableType::Vec4 };
	zxyw = VariableDummy{ this, "zxyw", VariableType::Vec4 };
	zxzx = VariableDummy{ this, "zxzx", VariableType::Vec4 };
	zxzy = VariableDummy{ this, "zxzy", VariableType::Vec4 };
	zxzz = VariableDummy{ this, "zxzz", VariableType::Vec4 };
	zxzw = VariableDummy{ this, "zxzw", VariableType::Vec4 };
	zxwx = VariableDummy{ this, "zxwx", VariableType::Vec4 };
	zxwy = VariableDummy{ this, "zxwy", VariableType::Vec4 };
	zxwz = VariableDummy{ this, "zxwz", VariableType::Vec4 };
	zxww = VariableDummy{ this, "zxww", VariableType::Vec4 };
	zyxx = VariableDummy{ this, "zyxx", VariableType::Vec4 };
	zyxy = VariableDummy{ this, "zyxy", VariableType::Vec4 };
	zyxz = VariableDummy{ this, "zyxz", VariableType::Vec4 };
	zyxw = VariableDummy{ this, "zyxw", VariableType::Vec4 };
	zyyx = VariableDummy{ this, "zyyx", VariableType::Vec4 };
	zyyy = VariableDummy{ this, "zyyy", VariableType::Vec4 };
	zyyz = VariableDummy{ this, "zyyz", VariableType::Vec4 };
	zyyw = VariableDummy{ this, "zyyw", VariableType::Vec4 };
	zyzx = VariableDummy{ this, "zyzx", VariableType::Vec4 };
	zyzy = VariableDummy{ this, "zyzy", VariableType::Vec4 };
	zyzz = VariableDummy{ this, "zyzz", VariableType::Vec4 };
	zyzw = VariableDummy{ this, "zyzw", VariableType::Vec4 };
	zywx = VariableDummy{ this, "zywx", VariableType::Vec4 };
	zywy = VariableDummy{ this, "zywy", VariableType::Vec4 };
	zywz = VariableDummy{ this, "zywz", VariableType::Vec4 };
	zyww = VariableDummy{ this, "zyww", VariableType::Vec4 };
	zzxx = VariableDummy{ this, "zzxx", VariableType::Vec4 };
	zzxy = VariableDummy{ this, "zzxy", VariableType::Vec4 };
	zzxz = VariableDummy{ this, "zzxz", VariableType::Vec4 };
	zzxw = VariableDummy{ this, "zzxw", VariableType::Vec4 };
	zzyx = VariableDummy{ this, "zzyx", VariableType::Vec4 };
	zzyy = VariableDummy{ this, "zzyy", VariableType::Vec4 };
	zzyz = VariableDummy{ this, "zzyz", VariableType::Vec4 };
	zzyw = VariableDummy{ this, "zzyw", VariableType::Vec4 };
	zzzx = VariableDummy{ this, "zzzx", VariableType::Vec4 };
	zzzy = VariableDummy{ this, "zzzy", VariableType::Vec4 };
	zzzz = VariableDummy{ this, "zzzz", VariableType::Vec4 };
	zzzw = VariableDummy{ this, "zzzw", VariableType::Vec4 };
	zzwx = VariableDummy{ this, "zzwx", VariableType::Vec4 };
	zzwy = VariableDummy{ this, "zzwy", VariableType::Vec4 };
	zzwz = VariableDummy{ this, "zzwz", VariableType::Vec4 };
	zzww = VariableDummy{ this, "zzww", VariableType::Vec4 };
	zwxx = VariableDummy{ this, "zwxx", VariableType::Vec4 };
	zwxy = VariableDummy{ this, "zwxy", VariableType::Vec4 };
	zwxz = VariableDummy{ this, "zwxz", VariableType::Vec4 };
	zwxw = VariableDummy{ this, "zwxw", VariableType::Vec4 };
	zwyx = VariableDummy{ this, "zwyx", VariableType::Vec4 };
	zwyy = VariableDummy{ this, "zwyy", VariableType::Vec4 };
	zwyz = VariableDummy{ this, "zwyz", VariableType::Vec4 };
	zwyw = VariableDummy{ this, "zwyw", VariableType::Vec4 };
	zwzx = VariableDummy{ this, "zwzx", VariableType::Vec4 };
	zwzy = VariableDummy{ this, "zwzy", VariableType::Vec4 };
	zwzz = VariableDummy{ this, "zwzz", VariableType::Vec4 };
	zwzw = VariableDummy{ this, "zwzw", VariableType::Vec4 };
	zwwx = VariableDummy{ this, "zwwx", VariableType::Vec4 };
	zwwy = VariableDummy{ this, "zwwy", VariableType::Vec4 };
	zwwz = VariableDummy{ this, "zwwz", VariableType::Vec4 };
	zwww = VariableDummy{ this, "zwww", VariableType::Vec4 };
	wxxx = VariableDummy{ this, "wxxx", VariableType::Vec4 };
	wxxy = VariableDummy{ this, "wxxy", VariableType::Vec4 };
	wxxz = VariableDummy{ this, "wxxz", VariableType::Vec4 };
	wxxw = VariableDummy{ this, "wxxw", VariableType::Vec4 };
	wxyx = VariableDummy{ this, "wxyx", VariableType::Vec4 };
	wxyy = VariableDummy{ this, "wxyy", VariableType::Vec4 };
	wxyz = VariableDummy{ this, "wxyz", VariableType::Vec4 };
	wxyw = VariableDummy{ this, "wxyw", VariableType::Vec4 };
	wxzx = VariableDummy{ this, "wxzx", VariableType::Vec4 };
	wxzy = VariableDummy{ this, "wxzy", VariableType::Vec4 };
	wxzz = VariableDummy{ this, "wxzz", VariableType::Vec4 };
	wxzw = VariableDummy{ this, "wxzw", VariableType::Vec4 };
	wxwx = VariableDummy{ this, "wxwx", VariableType::Vec4 };
	wxwy = VariableDummy{ this, "wxwy", VariableType::Vec4 };
	wxwz = VariableDummy{ this, "wxwz", VariableType::Vec4 };
	wxww = VariableDummy{ this, "wxww", VariableType::Vec4 };
	wyxx = VariableDummy{ this, "wyxx", VariableType::Vec4 };
	wyxy = VariableDummy{ this, "wyxy", VariableType::Vec4 };
	wyxz = VariableDummy{ this, "wyxz", VariableType::Vec4 };
	wyxw = VariableDummy{ this, "wyxw", VariableType::Vec4 };
	wyyx = VariableDummy{ this, "wyyx", VariableType::Vec4 };
	wyyy = VariableDummy{ this, "wyyy", VariableType::Vec4 };
	wyyz = VariableDummy{ this, "wyyz", VariableType::Vec4 };
	wyyw = VariableDummy{ this, "wyyw", VariableType::Vec4 };
	wyzx = VariableDummy{ this, "wyzx", VariableType::Vec4 };
	wyzy = VariableDummy{ this, "wyzy", VariableType::Vec4 };
	wyzz = VariableDummy{ this, "wyzz", VariableType::Vec4 };
	wyzw = VariableDummy{ this, "wyzw", VariableType::Vec4 };
	wywx = VariableDummy{ this, "wywx", VariableType::Vec4 };
	wywy = VariableDummy{ this, "wywy", VariableType::Vec4 };
	wywz = VariableDummy{ this, "wywz", VariableType::Vec4 };
	wyww = VariableDummy{ this, "wyww", VariableType::Vec4 };
	wzxx = VariableDummy{ this, "wzxx", VariableType::Vec4 };
	wzxy = VariableDummy{ this, "wzxy", VariableType::Vec4 };
	wzxz = VariableDummy{ this, "wzxz", VariableType::Vec4 };
	wzxw = VariableDummy{ this, "wzxw", VariableType::Vec4 };
	wzyx = VariableDummy{ this, "wzyx", VariableType::Vec4 };
	wzyy = VariableDummy{ this, "wzyy", VariableType::Vec4 };
	wzyz = VariableDummy{ this, "wzyz", VariableType::Vec4 };
	wzyw = VariableDummy{ this, "wzyw", VariableType::Vec4 };
	wzzx = VariableDummy{ this, "wzzx", VariableType::Vec4 };
	wzzy = VariableDummy{ this, "wzzy", VariableType::Vec4 };
	wzzz = VariableDummy{ this, "wzzz", VariableType::Vec4 };
	wzzw = VariableDummy{ this, "wzzw", VariableType::Vec4 };
	wzwx = VariableDummy{ this, "wzwx", VariableType::Vec4 };
	wzwy = VariableDummy{ this, "wzwy", VariableType::Vec4 };
	wzwz = VariableDummy{ this, "wzwz", VariableType::Vec4 };
	wzww = VariableDummy{ this, "wzww", VariableType::Vec4 };
	wwxx = VariableDummy{ this, "wwxx", VariableType::Vec4 };
	wwxy = VariableDummy{ this, "wwxy", VariableType::Vec4 };
	wwxz = VariableDummy{ this, "wwxz", VariableType::Vec4 };
	wwxw = VariableDummy{ this, "wwxw", VariableType::Vec4 };
	wwyx = VariableDummy{ this, "wwyx", VariableType::Vec4 };
	wwyy = VariableDummy{ this, "wwyy", VariableType::Vec4 };
	wwyz = VariableDummy{ this, "wwyz", VariableType::Vec4 };
	wwyw = VariableDummy{ this, "wwyw", VariableType::Vec4 };
	wwzx = VariableDummy{ this, "wwzx", VariableType::Vec4 };
	wwzy = VariableDummy{ this, "wwzy", VariableType::Vec4 };
	wwzz = VariableDummy{ this, "wwzz", VariableType::Vec4 };
	wwzw = VariableDummy{ this, "wwzw", VariableType::Vec4 };
	wwwx = VariableDummy{ this, "wwwx", VariableType::Vec4 };
	wwwy = VariableDummy{ this, "wwwy", VariableType::Vec4 };
	wwwz = VariableDummy{ this, "wwwz", VariableType::Vec4 };
	wwww = VariableDummy{ this, "wwww", VariableType::Vec4 };

	r    = VariableDummy{ this, "r", VariableType::Float };
	g    = VariableDummy{ this, "g", VariableType::Float };
	b    = VariableDummy{ this, "b", VariableType::Float };
	a    = VariableDummy{ this, "a", VariableType::Float };

	rr   = VariableDummy{ this, "rr", VariableType::Vec2 };
	rg   = VariableDummy{ this, "rg", VariableType::Vec2 };
	rb   = VariableDummy{ this, "rb", VariableType::Vec2 };
	ra   = VariableDummy{ this, "ra", VariableType::Vec2 };
	gr   = VariableDummy{ this, "gr", VariableType::Vec2 };
	gg   = VariableDummy{ this, "gg", VariableType::Vec2 };
	gb   = VariableDummy{ this, "gb", VariableType::Vec2 };
	ga   = VariableDummy{ this, "ga", VariableType::Vec2 };
	br   = VariableDummy{ this, "br", VariableType::Vec2 };
	bg   = VariableDummy{ this, "bg", VariableType::Vec2 };
	bb   = VariableDummy{ this, "bb", VariableType::Vec2 };
	ba   = VariableDummy{ this, "ba", VariableType::Vec2 };
	ar   = VariableDummy{ this, "ar", VariableType::Vec2 };
	ag   = VariableDummy{ this, "ag", VariableType::Vec2 };
	ab   = VariableDummy{ this, "ab", VariableType::Vec2 };
	aa   = VariableDummy{ this, "aa", VariableType::Vec2 };

	rrr  = VariableDummy{ this, "rrr", VariableType::Vec3 };
	rrg  = VariableDummy{ this, "rrg", VariableType::Vec3 };
	rrb  = VariableDummy{ this, "rrb", VariableType::Vec3 };
	rra  = VariableDummy{ this, "rra", VariableType::Vec3 };
	rgr  = VariableDummy{ this, "rgr", VariableType::Vec3 };
	rgg  = VariableDummy{ this, "rgg", VariableType::Vec3 };
	rgb  = VariableDummy{ this, "rgb", VariableType::Vec3 };
	rga  = VariableDummy{ this, "rga", VariableType::Vec3 };
	rbr  = VariableDummy{ this, "rbr", VariableType::Vec3 };
	rbg  = VariableDummy{ this, "rbg", VariableType::Vec3 };
	rbb  = VariableDummy{ this, "rbb", VariableType::Vec3 };
	rba  = VariableDummy{ this, "rba", VariableType::Vec3 };
	rar  = VariableDummy{ this, "rar", VariableType::Vec3 };
	rag  = VariableDummy{ this, "rag", VariableType::Vec3 };
	rab  = VariableDummy{ this, "rab", VariableType::Vec3 };
	raa  = VariableDummy{ this, "raa", VariableType::Vec3 };
	grr  = VariableDummy{ this, "grr", VariableType::Vec3 };
	grg  = VariableDummy{ this, "grg", VariableType::Vec3 };
	grb  = VariableDummy{ this, "grb", VariableType::Vec3 };
	gra  = VariableDummy{ this, "gra", VariableType::Vec3 };
	ggr  = VariableDummy{ this, "ggr", VariableType::Vec3 };
	ggg  = VariableDummy{ this, "ggg", VariableType::Vec3 };
	ggb  = VariableDummy{ this, "ggb", VariableType::Vec3 };
	gga  = VariableDummy{ this, "gga", VariableType::Vec3 };
	gbr  = VariableDummy{ this, "gbr", VariableType::Vec3 };
	gbg  = VariableDummy{ this, "gbg", VariableType::Vec3 };
	gbb  = VariableDummy{ this, "gbb", VariableType::Vec3 };
	gba  = VariableDummy{ this, "gba", VariableType::Vec3 };
	gar  = VariableDummy{ this, "gar", VariableType::Vec3 };
	gag  = VariableDummy{ this, "gag", VariableType::Vec3 };
	gab  = VariableDummy{ this, "gab", VariableType::Vec3 };
	gaa  = VariableDummy{ this, "gaa", VariableType::Vec3 };
	brr  = VariableDummy{ this, "brr", VariableType::Vec3 };
	brg  = VariableDummy{ this, "brg", VariableType::Vec3 };
	brb  = VariableDummy{ this, "brb", VariableType::Vec3 };
	bra  = VariableDummy{ this, "bra", VariableType::Vec3 };
	bgr  = VariableDummy{ this, "bgr", VariableType::Vec3 };
	bgg  = VariableDummy{ this, "bgg", VariableType::Vec3 };
	bgb  = VariableDummy{ this, "bgb", VariableType::Vec3 };
	bga  = VariableDummy{ this, "bga", VariableType::Vec3 };
	bbr  = VariableDummy{ this, "bbr", VariableType::Vec3 };
	bbg  = VariableDummy{ this, "bbg", VariableType::Vec3 };
	bbb  = VariableDummy{ this, "bbb", VariableType::Vec3 };
	bba  = VariableDummy{ this, "bba", VariableType::Vec3 };
	bar  = VariableDummy{ this, "bar", VariableType::Vec3 };
	bag  = VariableDummy{ this, "bag", VariableType::Vec3 };
	bab  = VariableDummy{ this, "bab", VariableType::Vec3 };
	baa  = VariableDummy{ this, "baa", VariableType::Vec3 };
	arr  = VariableDummy{ this, "arr", VariableType::Vec3 };
	arg  = VariableDummy{ this, "arg", VariableType::Vec3 };
	arb  = VariableDummy{ this, "arb", VariableType::Vec3 };
	ara  = VariableDummy{ this, "ara", VariableType::Vec3 };
	agr  = VariableDummy{ this, "agr", VariableType::Vec3 };
	agg  = VariableDummy{ this, "agg", VariableType::Vec3 };
	agb  = VariableDummy{ this, "agb", VariableType::Vec3 };
	aga  = VariableDummy{ this, "aga", VariableType::Vec3 };
	abr  = VariableDummy{ this, "abr", VariableType::Vec3 };
	abg  = VariableDummy{ this, "abg", VariableType::Vec3 };
	abb  = VariableDummy{ this, "abb", VariableType::Vec3 };
	aba  = VariableDummy{ this, "aba", VariableType::Vec3 };
	aar  = VariableDummy{ this, "aar", VariableType::Vec3 };
	aag  = VariableDummy{ this, "aag", VariableType::Vec3 };
	aab  = VariableDummy{ this, "aab", VariableType::Vec3 };
	aaa  = VariableDummy{ this, "aaa", VariableType::Vec3 };

	rrrr = VariableDummy{ this, "rrrr", VariableType::Vec4 };
	rrrg = VariableDummy{ this, "rrrg", VariableType::Vec4 };
	rrrb = VariableDummy{ this, "rrrb", VariableType::Vec4 };
	rrra = VariableDummy{ this, "rrra", VariableType::Vec4 };
	rrgr = VariableDummy{ this, "rrgr", VariableType::Vec4 };
	rrgg = VariableDummy{ this, "rrgg", VariableType::Vec4 };
	rrgb = VariableDummy{ this, "rrgb", VariableType::Vec4 };
	rrga = VariableDummy{ this, "rrga", VariableType::Vec4 };
	rrbr = VariableDummy{ this, "rrbr", VariableType::Vec4 };
	rrbg = VariableDummy{ this, "rrbg", VariableType::Vec4 };
	rrbb = VariableDummy{ this, "rrbb", VariableType::Vec4 };
	rrba = VariableDummy{ this, "rrba", VariableType::Vec4 };
	rrar = VariableDummy{ this, "rrar", VariableType::Vec4 };
	rrag = VariableDummy{ this, "rrag", VariableType::Vec4 };
	rrab = VariableDummy{ this, "rrab", VariableType::Vec4 };
	rraa = VariableDummy{ this, "rraa", VariableType::Vec4 };
	rgrr = VariableDummy{ this, "rgrr", VariableType::Vec4 };
	rgrg = VariableDummy{ this, "rgrg", VariableType::Vec4 };
	rgrb = VariableDummy{ this, "rgrb", VariableType::Vec4 };
	rgra = VariableDummy{ this, "rgra", VariableType::Vec4 };
	rggr = VariableDummy{ this, "rggr", VariableType::Vec4 };
	rggg = VariableDummy{ this, "rggg", VariableType::Vec4 };
	rggb = VariableDummy{ this, "rggb", VariableType::Vec4 };
	rgga = VariableDummy{ this, "rgga", VariableType::Vec4 };
	rgbr = VariableDummy{ this, "rgbr", VariableType::Vec4 };
	rgbg = VariableDummy{ this, "rgbg", VariableType::Vec4 };
	rgbb = VariableDummy{ this, "rgbb", VariableType::Vec4 };
	rgba = VariableDummy{ this, "rgba", VariableType::Vec4 };
	rgar = VariableDummy{ this, "rgar", VariableType::Vec4 };
	rgag = VariableDummy{ this, "rgag", VariableType::Vec4 };
	rgab = VariableDummy{ this, "rgab", VariableType::Vec4 };
	rgaa = VariableDummy{ this, "rgaa", VariableType::Vec4 };
	rbrr = VariableDummy{ this, "rbrr", VariableType::Vec4 };
	rbrg = VariableDummy{ this, "rbrg", VariableType::Vec4 };
	rbrb = VariableDummy{ this, "rbrb", VariableType::Vec4 };
	rbra = VariableDummy{ this, "rbra", VariableType::Vec4 };
	rbgr = VariableDummy{ this, "rbgr", VariableType::Vec4 };
	rbgg = VariableDummy{ this, "rbgg", VariableType::Vec4 };
	rbgb = VariableDummy{ this, "rbgb", VariableType::Vec4 };
	rbga = VariableDummy{ this, "rbga", VariableType::Vec4 };
	rbbr = VariableDummy{ this, "rbbr", VariableType::Vec4 };
	rbbg = VariableDummy{ this, "rbbg", VariableType::Vec4 };
	rbbb = VariableDummy{ this, "rbbb", VariableType::Vec4 };
	rbba = VariableDummy{ this, "rbba", VariableType::Vec4 };
	rbar = VariableDummy{ this, "rbar", VariableType::Vec4 };
	rbag = VariableDummy{ this, "rbag", VariableType::Vec4 };
	rbab = VariableDummy{ this, "rbab", VariableType::Vec4 };
	rbaa = VariableDummy{ this, "rbaa", VariableType::Vec4 };
	rarr = VariableDummy{ this, "rarr", VariableType::Vec4 };
	rarg = VariableDummy{ this, "rarg", VariableType::Vec4 };
	rarb = VariableDummy{ this, "rarb", VariableType::Vec4 };
	rara = VariableDummy{ this, "rara", VariableType::Vec4 };
	ragr = VariableDummy{ this, "ragr", VariableType::Vec4 };
	ragg = VariableDummy{ this, "ragg", VariableType::Vec4 };
	ragb = VariableDummy{ this, "ragb", VariableType::Vec4 };
	raga = VariableDummy{ this, "raga", VariableType::Vec4 };
	rabr = VariableDummy{ this, "rabr", VariableType::Vec4 };
	rabg = VariableDummy{ this, "rabg", VariableType::Vec4 };
	rabb = VariableDummy{ this, "rabb", VariableType::Vec4 };
	raba = VariableDummy{ this, "raba", VariableType::Vec4 };
	raar = VariableDummy{ this, "raar", VariableType::Vec4 };
	raag = VariableDummy{ this, "raag", VariableType::Vec4 };
	raab = VariableDummy{ this, "raab", VariableType::Vec4 };
	raaa = VariableDummy{ this, "raaa", VariableType::Vec4 };
	grrr = VariableDummy{ this, "grrr", VariableType::Vec4 };
	grrg = VariableDummy{ this, "grrg", VariableType::Vec4 };
	grrb = VariableDummy{ this, "grrb", VariableType::Vec4 };
	grra = VariableDummy{ this, "grra", VariableType::Vec4 };
	grgr = VariableDummy{ this, "grgr", VariableType::Vec4 };
	grgg = VariableDummy{ this, "grgg", VariableType::Vec4 };
	grgb = VariableDummy{ this, "grgb", VariableType::Vec4 };
	grga = VariableDummy{ this, "grga", VariableType::Vec4 };
	grbr = VariableDummy{ this, "grbr", VariableType::Vec4 };
	grbg = VariableDummy{ this, "grbg", VariableType::Vec4 };
	grbb = VariableDummy{ this, "grbb", VariableType::Vec4 };
	grba = VariableDummy{ this, "grba", VariableType::Vec4 };
	grar = VariableDummy{ this, "grar", VariableType::Vec4 };
	grag = VariableDummy{ this, "grag", VariableType::Vec4 };
	grab = VariableDummy{ this, "grab", VariableType::Vec4 };
	graa = VariableDummy{ this, "graa", VariableType::Vec4 };
	ggrr = VariableDummy{ this, "ggrr", VariableType::Vec4 };
	ggrg = VariableDummy{ this, "ggrg", VariableType::Vec4 };
	ggrb = VariableDummy{ this, "ggrb", VariableType::Vec4 };
	ggra = VariableDummy{ this, "ggra", VariableType::Vec4 };
	gggr = VariableDummy{ this, "gggr", VariableType::Vec4 };
	gggg = VariableDummy{ this, "gggg", VariableType::Vec4 };
	gggb = VariableDummy{ this, "gggb", VariableType::Vec4 };
	ggga = VariableDummy{ this, "ggga", VariableType::Vec4 };
	ggbr = VariableDummy{ this, "ggbr", VariableType::Vec4 };
	ggbg = VariableDummy{ this, "ggbg", VariableType::Vec4 };
	ggbb = VariableDummy{ this, "ggbb", VariableType::Vec4 };
	ggba = VariableDummy{ this, "ggba", VariableType::Vec4 };
	ggar = VariableDummy{ this, "ggar", VariableType::Vec4 };
	ggag = VariableDummy{ this, "ggag", VariableType::Vec4 };
	ggab = VariableDummy{ this, "ggab", VariableType::Vec4 };
	ggaa = VariableDummy{ this, "ggaa", VariableType::Vec4 };
	gbrr = VariableDummy{ this, "gbrr", VariableType::Vec4 };
	gbrg = VariableDummy{ this, "gbrg", VariableType::Vec4 };
	gbrb = VariableDummy{ this, "gbrb", VariableType::Vec4 };
	gbra = VariableDummy{ this, "gbra", VariableType::Vec4 };
	gbgr = VariableDummy{ this, "gbgr", VariableType::Vec4 };
	gbgg = VariableDummy{ this, "gbgg", VariableType::Vec4 };
	gbgb = VariableDummy{ this, "gbgb", VariableType::Vec4 };
	gbga = VariableDummy{ this, "gbga", VariableType::Vec4 };
	gbbr = VariableDummy{ this, "gbbr", VariableType::Vec4 };
	gbbg = VariableDummy{ this, "gbbg", VariableType::Vec4 };
	gbbb = VariableDummy{ this, "gbbb", VariableType::Vec4 };
	gbba = VariableDummy{ this, "gbba", VariableType::Vec4 };
	gbar = VariableDummy{ this, "gbar", VariableType::Vec4 };
	gbag = VariableDummy{ this, "gbag", VariableType::Vec4 };
	gbab = VariableDummy{ this, "gbab", VariableType::Vec4 };
	gbaa = VariableDummy{ this, "gbaa", VariableType::Vec4 };
	garr = VariableDummy{ this, "garr", VariableType::Vec4 };
	garg = VariableDummy{ this, "garg", VariableType::Vec4 };
	garb = VariableDummy{ this, "garb", VariableType::Vec4 };
	gara = VariableDummy{ this, "gara", VariableType::Vec4 };
	gagr = VariableDummy{ this, "gagr", VariableType::Vec4 };
	gagg = VariableDummy{ this, "gagg", VariableType::Vec4 };
	gagb = VariableDummy{ this, "gagb", VariableType::Vec4 };
	gaga = VariableDummy{ this, "gaga", VariableType::Vec4 };
	gabr = VariableDummy{ this, "gabr", VariableType::Vec4 };
	gabg = VariableDummy{ this, "gabg", VariableType::Vec4 };
	gabb = VariableDummy{ this, "gabb", VariableType::Vec4 };
	gaba = VariableDummy{ this, "gaba", VariableType::Vec4 };
	gaar = VariableDummy{ this, "gaar", VariableType::Vec4 };
	gaag = VariableDummy{ this, "gaag", VariableType::Vec4 };
	gaab = VariableDummy{ this, "gaab", VariableType::Vec4 };
	gaaa = VariableDummy{ this, "gaaa", VariableType::Vec4 };
	brrr = VariableDummy{ this, "brrr", VariableType::Vec4 };
	brrg = VariableDummy{ this, "brrg", VariableType::Vec4 };
	brrb = VariableDummy{ this, "brrb", VariableType::Vec4 };
	brra = VariableDummy{ this, "brra", VariableType::Vec4 };
	brgr = VariableDummy{ this, "brgr", VariableType::Vec4 };
	brgg = VariableDummy{ this, "brgg", VariableType::Vec4 };
	brgb = VariableDummy{ this, "brgb", VariableType::Vec4 };
	brga = VariableDummy{ this, "brga", VariableType::Vec4 };
	brbr = VariableDummy{ this, "brbr", VariableType::Vec4 };
	brbg = VariableDummy{ this, "brbg", VariableType::Vec4 };
	brbb = VariableDummy{ this, "brbb", VariableType::Vec4 };
	brba = VariableDummy{ this, "brba", VariableType::Vec4 };
	brar = VariableDummy{ this, "brar", VariableType::Vec4 };
	brag = VariableDummy{ this, "brag", VariableType::Vec4 };
	brab = VariableDummy{ this, "brab", VariableType::Vec4 };
	braa = VariableDummy{ this, "braa", VariableType::Vec4 };
	bgrr = VariableDummy{ this, "bgrr", VariableType::Vec4 };
	bgrg = VariableDummy{ this, "bgrg", VariableType::Vec4 };
	bgrb = VariableDummy{ this, "bgrb", VariableType::Vec4 };
	bgra = VariableDummy{ this, "bgra", VariableType::Vec4 };
	bggr = VariableDummy{ this, "bggr", VariableType::Vec4 };
	bggg = VariableDummy{ this, "bggg", VariableType::Vec4 };
	bggb = VariableDummy{ this, "bggb", VariableType::Vec4 };
	bgga = VariableDummy{ this, "bgga", VariableType::Vec4 };
	bgbr = VariableDummy{ this, "bgbr", VariableType::Vec4 };
	bgbg = VariableDummy{ this, "bgbg", VariableType::Vec4 };
	bgbb = VariableDummy{ this, "bgbb", VariableType::Vec4 };
	bgba = VariableDummy{ this, "bgba", VariableType::Vec4 };
	bgar = VariableDummy{ this, "bgar", VariableType::Vec4 };
	bgag = VariableDummy{ this, "bgag", VariableType::Vec4 };
	bgab = VariableDummy{ this, "bgab", VariableType::Vec4 };
	bgaa = VariableDummy{ this, "bgaa", VariableType::Vec4 };
	bbrr = VariableDummy{ this, "bbrr", VariableType::Vec4 };
	bbrg = VariableDummy{ this, "bbrg", VariableType::Vec4 };
	bbrb = VariableDummy{ this, "bbrb", VariableType::Vec4 };
	bbra = VariableDummy{ this, "bbra", VariableType::Vec4 };
	bbgr = VariableDummy{ this, "bbgr", VariableType::Vec4 };
	bbgg = VariableDummy{ this, "bbgg", VariableType::Vec4 };
	bbgb = VariableDummy{ this, "bbgb", VariableType::Vec4 };
	bbga = VariableDummy{ this, "bbga", VariableType::Vec4 };
	bbbr = VariableDummy{ this, "bbbr", VariableType::Vec4 };
	bbbg = VariableDummy{ this, "bbbg", VariableType::Vec4 };
	bbbb = VariableDummy{ this, "bbbb", VariableType::Vec4 };
	bbba = VariableDummy{ this, "bbba", VariableType::Vec4 };
	bbar = VariableDummy{ this, "bbar", VariableType::Vec4 };
	bbag = VariableDummy{ this, "bbag", VariableType::Vec4 };
	bbab = VariableDummy{ this, "bbab", VariableType::Vec4 };
	bbaa = VariableDummy{ this, "bbaa", VariableType::Vec4 };
	barr = VariableDummy{ this, "barr", VariableType::Vec4 };
	barg = VariableDummy{ this, "barg", VariableType::Vec4 };
	barb = VariableDummy{ this, "barb", VariableType::Vec4 };
	bara = VariableDummy{ this, "bara", VariableType::Vec4 };
	bagr = VariableDummy{ this, "bagr", VariableType::Vec4 };
	bagg = VariableDummy{ this, "bagg", VariableType::Vec4 };
	bagb = VariableDummy{ this, "bagb", VariableType::Vec4 };
	baga = VariableDummy{ this, "baga", VariableType::Vec4 };
	babr = VariableDummy{ this, "babr", VariableType::Vec4 };
	babg = VariableDummy{ this, "babg", VariableType::Vec4 };
	babb = VariableDummy{ this, "babb", VariableType::Vec4 };
	baba = VariableDummy{ this, "baba", VariableType::Vec4 };
	baar = VariableDummy{ this, "baar", VariableType::Vec4 };
	baag = VariableDummy{ this, "baag", VariableType::Vec4 };
	baab = VariableDummy{ this, "baab", VariableType::Vec4 };
	baaa = VariableDummy{ this, "baaa", VariableType::Vec4 };
	arrr = VariableDummy{ this, "arrr", VariableType::Vec4 };
	arrg = VariableDummy{ this, "arrg", VariableType::Vec4 };
	arrb = VariableDummy{ this, "arrb", VariableType::Vec4 };
	arra = VariableDummy{ this, "arra", VariableType::Vec4 };
	argr = VariableDummy{ this, "argr", VariableType::Vec4 };
	argg = VariableDummy{ this, "argg", VariableType::Vec4 };
	argb = VariableDummy{ this, "argb", VariableType::Vec4 };
	arga = VariableDummy{ this, "arga", VariableType::Vec4 };
	arbr = VariableDummy{ this, "arbr", VariableType::Vec4 };
	arbg = VariableDummy{ this, "arbg", VariableType::Vec4 };
	arbb = VariableDummy{ this, "arbb", VariableType::Vec4 };
	arba = VariableDummy{ this, "arba", VariableType::Vec4 };
	arar = VariableDummy{ this, "arar", VariableType::Vec4 };
	arag = VariableDummy{ this, "arag", VariableType::Vec4 };
	arab = VariableDummy{ this, "arab", VariableType::Vec4 };
	araa = VariableDummy{ this, "araa", VariableType::Vec4 };
	agrr = VariableDummy{ this, "agrr", VariableType::Vec4 };
	agrg = VariableDummy{ this, "agrg", VariableType::Vec4 };
	agrb = VariableDummy{ this, "agrb", VariableType::Vec4 };
	agra = VariableDummy{ this, "agra", VariableType::Vec4 };
	aggr = VariableDummy{ this, "aggr", VariableType::Vec4 };
	aggg = VariableDummy{ this, "aggg", VariableType::Vec4 };
	aggb = VariableDummy{ this, "aggb", VariableType::Vec4 };
	agga = VariableDummy{ this, "agga", VariableType::Vec4 };
	agbr = VariableDummy{ this, "agbr", VariableType::Vec4 };
	agbg = VariableDummy{ this, "agbg", VariableType::Vec4 };
	agbb = VariableDummy{ this, "agbb", VariableType::Vec4 };
	agba = VariableDummy{ this, "agba", VariableType::Vec4 };
	agar = VariableDummy{ this, "agar", VariableType::Vec4 };
	agag = VariableDummy{ this, "agag", VariableType::Vec4 };
	agab = VariableDummy{ this, "agab", VariableType::Vec4 };
	agaa = VariableDummy{ this, "agaa", VariableType::Vec4 };
	abrr = VariableDummy{ this, "abrr", VariableType::Vec4 };
	abrg = VariableDummy{ this, "abrg", VariableType::Vec4 };
	abrb = VariableDummy{ this, "abrb", VariableType::Vec4 };
	abra = VariableDummy{ this, "abra", VariableType::Vec4 };
	abgr = VariableDummy{ this, "abgr", VariableType::Vec4 };
	abgg = VariableDummy{ this, "abgg", VariableType::Vec4 };
	abgb = VariableDummy{ this, "abgb", VariableType::Vec4 };
	abga = VariableDummy{ this, "abga", VariableType::Vec4 };
	abbr = VariableDummy{ this, "abbr", VariableType::Vec4 };
	abbg = VariableDummy{ this, "abbg", VariableType::Vec4 };
	abbb = VariableDummy{ this, "abbb", VariableType::Vec4 };
	abba = VariableDummy{ this, "abba", VariableType::Vec4 };
	abar = VariableDummy{ this, "abar", VariableType::Vec4 };
	abag = VariableDummy{ this, "abag", VariableType::Vec4 };
	abab = VariableDummy{ this, "abab", VariableType::Vec4 };
	abaa = VariableDummy{ this, "abaa", VariableType::Vec4 };
	aarr = VariableDummy{ this, "aarr", VariableType::Vec4 };
	aarg = VariableDummy{ this, "aarg", VariableType::Vec4 };
	aarb = VariableDummy{ this, "aarb", VariableType::Vec4 };
	aara = VariableDummy{ this, "aara", VariableType::Vec4 };
	aagr = VariableDummy{ this, "aagr", VariableType::Vec4 };
	aagg = VariableDummy{ this, "aagg", VariableType::Vec4 };
	aagb = VariableDummy{ this, "aagb", VariableType::Vec4 };
	aaga = VariableDummy{ this, "aaga", VariableType::Vec4 };
	aabr = VariableDummy{ this, "aabr", VariableType::Vec4 };
	aabg = VariableDummy{ this, "aabg", VariableType::Vec4 };
	aabb = VariableDummy{ this, "aabb", VariableType::Vec4 };
	aaba = VariableDummy{ this, "aaba", VariableType::Vec4 };
	aaar = VariableDummy{ this, "aaar", VariableType::Vec4 };
	aaag = VariableDummy{ this, "aaag", VariableType::Vec4 };
	aaab = VariableDummy{ this, "aaab", VariableType::Vec4 };
	aaaa = VariableDummy{ this, "aaaa", VariableType::Vec4 };
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
