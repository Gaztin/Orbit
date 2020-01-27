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
	InitProxies();
}

ShaderInterface::Variable::Variable( const Variable& other )
	: m_value( other.m_value )
{
	other.m_used = true;

	InitProxies();
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

	InitProxies();
}

ShaderInterface::Variable::Variable( double value )
{
	std::ostringstream ss;
	ss << std::fixed << value;
	m_value = ss.str();

	InitProxies();
}

ShaderInterface::Variable::Variable( std::string_view name, VariableType type )
	: m_value( name )
	, m_type ( type )
{
	InitProxies();
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

void ShaderInterface::Variable::InitProxies( void )
{
	x    = { this, VariableType::Float };
	y    = { this, VariableType::Float };
	z    = { this, VariableType::Float };
	w    = { this, VariableType::Float };

	xx   = { this, VariableType::Vec2 };
	xy   = { this, VariableType::Vec2 };
	xz   = { this, VariableType::Vec2 };
	xw   = { this, VariableType::Vec2 };
	yx   = { this, VariableType::Vec2 };
	yy   = { this, VariableType::Vec2 };
	yz   = { this, VariableType::Vec2 };
	yw   = { this, VariableType::Vec2 };
	zx   = { this, VariableType::Vec2 };
	zy   = { this, VariableType::Vec2 };
	zz   = { this, VariableType::Vec2 };
	zw   = { this, VariableType::Vec2 };
	wx   = { this, VariableType::Vec2 };
	wy   = { this, VariableType::Vec2 };
	wz   = { this, VariableType::Vec2 };
	ww   = { this, VariableType::Vec2 };

	xxx  = { this, VariableType::Vec3 };
	xxy  = { this, VariableType::Vec3 };
	xxz  = { this, VariableType::Vec3 };
	xxw  = { this, VariableType::Vec3 };
	xyx  = { this, VariableType::Vec3 };
	xyy  = { this, VariableType::Vec3 };
	xyz  = { this, VariableType::Vec3 };
	xyw  = { this, VariableType::Vec3 };
	xzx  = { this, VariableType::Vec3 };
	xzy  = { this, VariableType::Vec3 };
	xzz  = { this, VariableType::Vec3 };
	xzw  = { this, VariableType::Vec3 };
	xwx  = { this, VariableType::Vec3 };
	xwy  = { this, VariableType::Vec3 };
	xwz  = { this, VariableType::Vec3 };
	xww  = { this, VariableType::Vec3 };
	yxx  = { this, VariableType::Vec3 };
	yxy  = { this, VariableType::Vec3 };
	yxz  = { this, VariableType::Vec3 };
	yxw  = { this, VariableType::Vec3 };
	yyx  = { this, VariableType::Vec3 };
	yyy  = { this, VariableType::Vec3 };
	yyz  = { this, VariableType::Vec3 };
	yyw  = { this, VariableType::Vec3 };
	yzx  = { this, VariableType::Vec3 };
	yzy  = { this, VariableType::Vec3 };
	yzz  = { this, VariableType::Vec3 };
	yzw  = { this, VariableType::Vec3 };
	ywx  = { this, VariableType::Vec3 };
	ywy  = { this, VariableType::Vec3 };
	ywz  = { this, VariableType::Vec3 };
	yww  = { this, VariableType::Vec3 };
	zxx  = { this, VariableType::Vec3 };
	zxy  = { this, VariableType::Vec3 };
	zxz  = { this, VariableType::Vec3 };
	zxw  = { this, VariableType::Vec3 };
	zyx  = { this, VariableType::Vec3 };
	zyy  = { this, VariableType::Vec3 };
	zyz  = { this, VariableType::Vec3 };
	zyw  = { this, VariableType::Vec3 };
	zzx  = { this, VariableType::Vec3 };
	zzy  = { this, VariableType::Vec3 };
	zzz  = { this, VariableType::Vec3 };
	zzw  = { this, VariableType::Vec3 };
	zwx  = { this, VariableType::Vec3 };
	zwy  = { this, VariableType::Vec3 };
	zwz  = { this, VariableType::Vec3 };
	zww  = { this, VariableType::Vec3 };
	wxx  = { this, VariableType::Vec3 };
	wxy  = { this, VariableType::Vec3 };
	wxz  = { this, VariableType::Vec3 };
	wxw  = { this, VariableType::Vec3 };
	wyx  = { this, VariableType::Vec3 };
	wyy  = { this, VariableType::Vec3 };
	wyz  = { this, VariableType::Vec3 };
	wyw  = { this, VariableType::Vec3 };
	wzx  = { this, VariableType::Vec3 };
	wzy  = { this, VariableType::Vec3 };
	wzz  = { this, VariableType::Vec3 };
	wzw  = { this, VariableType::Vec3 };
	wwx  = { this, VariableType::Vec3 };
	wwy  = { this, VariableType::Vec3 };
	wwz  = { this, VariableType::Vec3 };
	www  = { this, VariableType::Vec3 };

	xxxx = { this, VariableType::Vec4 };
	xxxy = { this, VariableType::Vec4 };
	xxxz = { this, VariableType::Vec4 };
	xxxw = { this, VariableType::Vec4 };
	xxyx = { this, VariableType::Vec4 };
	xxyy = { this, VariableType::Vec4 };
	xxyz = { this, VariableType::Vec4 };
	xxyw = { this, VariableType::Vec4 };
	xxzx = { this, VariableType::Vec4 };
	xxzy = { this, VariableType::Vec4 };
	xxzz = { this, VariableType::Vec4 };
	xxzw = { this, VariableType::Vec4 };
	xxwx = { this, VariableType::Vec4 };
	xxwy = { this, VariableType::Vec4 };
	xxwz = { this, VariableType::Vec4 };
	xxww = { this, VariableType::Vec4 };
	xyxx = { this, VariableType::Vec4 };
	xyxy = { this, VariableType::Vec4 };
	xyxz = { this, VariableType::Vec4 };
	xyxw = { this, VariableType::Vec4 };
	xyyx = { this, VariableType::Vec4 };
	xyyy = { this, VariableType::Vec4 };
	xyyz = { this, VariableType::Vec4 };
	xyyw = { this, VariableType::Vec4 };
	xyzx = { this, VariableType::Vec4 };
	xyzy = { this, VariableType::Vec4 };
	xyzz = { this, VariableType::Vec4 };
	xyzw = { this, VariableType::Vec4 };
	xywx = { this, VariableType::Vec4 };
	xywy = { this, VariableType::Vec4 };
	xywz = { this, VariableType::Vec4 };
	xyww = { this, VariableType::Vec4 };
	xzxx = { this, VariableType::Vec4 };
	xzxy = { this, VariableType::Vec4 };
	xzxz = { this, VariableType::Vec4 };
	xzxw = { this, VariableType::Vec4 };
	xzyx = { this, VariableType::Vec4 };
	xzyy = { this, VariableType::Vec4 };
	xzyz = { this, VariableType::Vec4 };
	xzyw = { this, VariableType::Vec4 };
	xzzx = { this, VariableType::Vec4 };
	xzzy = { this, VariableType::Vec4 };
	xzzz = { this, VariableType::Vec4 };
	xzzw = { this, VariableType::Vec4 };
	xzwx = { this, VariableType::Vec4 };
	xzwy = { this, VariableType::Vec4 };
	xzwz = { this, VariableType::Vec4 };
	xzww = { this, VariableType::Vec4 };
	xwxx = { this, VariableType::Vec4 };
	xwxy = { this, VariableType::Vec4 };
	xwxz = { this, VariableType::Vec4 };
	xwxw = { this, VariableType::Vec4 };
	xwyx = { this, VariableType::Vec4 };
	xwyy = { this, VariableType::Vec4 };
	xwyz = { this, VariableType::Vec4 };
	xwyw = { this, VariableType::Vec4 };
	xwzx = { this, VariableType::Vec4 };
	xwzy = { this, VariableType::Vec4 };
	xwzz = { this, VariableType::Vec4 };
	xwzw = { this, VariableType::Vec4 };
	xwwx = { this, VariableType::Vec4 };
	xwwy = { this, VariableType::Vec4 };
	xwwz = { this, VariableType::Vec4 };
	xwww = { this, VariableType::Vec4 };
	yxxx = { this, VariableType::Vec4 };
	yxxy = { this, VariableType::Vec4 };
	yxxz = { this, VariableType::Vec4 };
	yxxw = { this, VariableType::Vec4 };
	yxyx = { this, VariableType::Vec4 };
	yxyy = { this, VariableType::Vec4 };
	yxyz = { this, VariableType::Vec4 };
	yxyw = { this, VariableType::Vec4 };
	yxzx = { this, VariableType::Vec4 };
	yxzy = { this, VariableType::Vec4 };
	yxzz = { this, VariableType::Vec4 };
	yxzw = { this, VariableType::Vec4 };
	yxwx = { this, VariableType::Vec4 };
	yxwy = { this, VariableType::Vec4 };
	yxwz = { this, VariableType::Vec4 };
	yxww = { this, VariableType::Vec4 };
	yyxx = { this, VariableType::Vec4 };
	yyxy = { this, VariableType::Vec4 };
	yyxz = { this, VariableType::Vec4 };
	yyxw = { this, VariableType::Vec4 };
	yyyx = { this, VariableType::Vec4 };
	yyyy = { this, VariableType::Vec4 };
	yyyz = { this, VariableType::Vec4 };
	yyyw = { this, VariableType::Vec4 };
	yyzx = { this, VariableType::Vec4 };
	yyzy = { this, VariableType::Vec4 };
	yyzz = { this, VariableType::Vec4 };
	yyzw = { this, VariableType::Vec4 };
	yywx = { this, VariableType::Vec4 };
	yywy = { this, VariableType::Vec4 };
	yywz = { this, VariableType::Vec4 };
	yyww = { this, VariableType::Vec4 };
	yzxx = { this, VariableType::Vec4 };
	yzxy = { this, VariableType::Vec4 };
	yzxz = { this, VariableType::Vec4 };
	yzxw = { this, VariableType::Vec4 };
	yzyx = { this, VariableType::Vec4 };
	yzyy = { this, VariableType::Vec4 };
	yzyz = { this, VariableType::Vec4 };
	yzyw = { this, VariableType::Vec4 };
	yzzx = { this, VariableType::Vec4 };
	yzzy = { this, VariableType::Vec4 };
	yzzz = { this, VariableType::Vec4 };
	yzzw = { this, VariableType::Vec4 };
	yzwx = { this, VariableType::Vec4 };
	yzwy = { this, VariableType::Vec4 };
	yzwz = { this, VariableType::Vec4 };
	yzww = { this, VariableType::Vec4 };
	ywxx = { this, VariableType::Vec4 };
	ywxy = { this, VariableType::Vec4 };
	ywxz = { this, VariableType::Vec4 };
	ywxw = { this, VariableType::Vec4 };
	ywyx = { this, VariableType::Vec4 };
	ywyy = { this, VariableType::Vec4 };
	ywyz = { this, VariableType::Vec4 };
	ywyw = { this, VariableType::Vec4 };
	ywzx = { this, VariableType::Vec4 };
	ywzy = { this, VariableType::Vec4 };
	ywzz = { this, VariableType::Vec4 };
	ywzw = { this, VariableType::Vec4 };
	ywwx = { this, VariableType::Vec4 };
	ywwy = { this, VariableType::Vec4 };
	ywwz = { this, VariableType::Vec4 };
	ywww = { this, VariableType::Vec4 };
	zxxx = { this, VariableType::Vec4 };
	zxxy = { this, VariableType::Vec4 };
	zxxz = { this, VariableType::Vec4 };
	zxxw = { this, VariableType::Vec4 };
	zxyx = { this, VariableType::Vec4 };
	zxyy = { this, VariableType::Vec4 };
	zxyz = { this, VariableType::Vec4 };
	zxyw = { this, VariableType::Vec4 };
	zxzx = { this, VariableType::Vec4 };
	zxzy = { this, VariableType::Vec4 };
	zxzz = { this, VariableType::Vec4 };
	zxzw = { this, VariableType::Vec4 };
	zxwx = { this, VariableType::Vec4 };
	zxwy = { this, VariableType::Vec4 };
	zxwz = { this, VariableType::Vec4 };
	zxww = { this, VariableType::Vec4 };
	zyxx = { this, VariableType::Vec4 };
	zyxy = { this, VariableType::Vec4 };
	zyxz = { this, VariableType::Vec4 };
	zyxw = { this, VariableType::Vec4 };
	zyyx = { this, VariableType::Vec4 };
	zyyy = { this, VariableType::Vec4 };
	zyyz = { this, VariableType::Vec4 };
	zyyw = { this, VariableType::Vec4 };
	zyzx = { this, VariableType::Vec4 };
	zyzy = { this, VariableType::Vec4 };
	zyzz = { this, VariableType::Vec4 };
	zyzw = { this, VariableType::Vec4 };
	zywx = { this, VariableType::Vec4 };
	zywy = { this, VariableType::Vec4 };
	zywz = { this, VariableType::Vec4 };
	zyww = { this, VariableType::Vec4 };
	zzxx = { this, VariableType::Vec4 };
	zzxy = { this, VariableType::Vec4 };
	zzxz = { this, VariableType::Vec4 };
	zzxw = { this, VariableType::Vec4 };
	zzyx = { this, VariableType::Vec4 };
	zzyy = { this, VariableType::Vec4 };
	zzyz = { this, VariableType::Vec4 };
	zzyw = { this, VariableType::Vec4 };
	zzzx = { this, VariableType::Vec4 };
	zzzy = { this, VariableType::Vec4 };
	zzzz = { this, VariableType::Vec4 };
	zzzw = { this, VariableType::Vec4 };
	zzwx = { this, VariableType::Vec4 };
	zzwy = { this, VariableType::Vec4 };
	zzwz = { this, VariableType::Vec4 };
	zzww = { this, VariableType::Vec4 };
	zwxx = { this, VariableType::Vec4 };
	zwxy = { this, VariableType::Vec4 };
	zwxz = { this, VariableType::Vec4 };
	zwxw = { this, VariableType::Vec4 };
	zwyx = { this, VariableType::Vec4 };
	zwyy = { this, VariableType::Vec4 };
	zwyz = { this, VariableType::Vec4 };
	zwyw = { this, VariableType::Vec4 };
	zwzx = { this, VariableType::Vec4 };
	zwzy = { this, VariableType::Vec4 };
	zwzz = { this, VariableType::Vec4 };
	zwzw = { this, VariableType::Vec4 };
	zwwx = { this, VariableType::Vec4 };
	zwwy = { this, VariableType::Vec4 };
	zwwz = { this, VariableType::Vec4 };
	zwww = { this, VariableType::Vec4 };
	wxxx = { this, VariableType::Vec4 };
	wxxy = { this, VariableType::Vec4 };
	wxxz = { this, VariableType::Vec4 };
	wxxw = { this, VariableType::Vec4 };
	wxyx = { this, VariableType::Vec4 };
	wxyy = { this, VariableType::Vec4 };
	wxyz = { this, VariableType::Vec4 };
	wxyw = { this, VariableType::Vec4 };
	wxzx = { this, VariableType::Vec4 };
	wxzy = { this, VariableType::Vec4 };
	wxzz = { this, VariableType::Vec4 };
	wxzw = { this, VariableType::Vec4 };
	wxwx = { this, VariableType::Vec4 };
	wxwy = { this, VariableType::Vec4 };
	wxwz = { this, VariableType::Vec4 };
	wxww = { this, VariableType::Vec4 };
	wyxx = { this, VariableType::Vec4 };
	wyxy = { this, VariableType::Vec4 };
	wyxz = { this, VariableType::Vec4 };
	wyxw = { this, VariableType::Vec4 };
	wyyx = { this, VariableType::Vec4 };
	wyyy = { this, VariableType::Vec4 };
	wyyz = { this, VariableType::Vec4 };
	wyyw = { this, VariableType::Vec4 };
	wyzx = { this, VariableType::Vec4 };
	wyzy = { this, VariableType::Vec4 };
	wyzz = { this, VariableType::Vec4 };
	wyzw = { this, VariableType::Vec4 };
	wywx = { this, VariableType::Vec4 };
	wywy = { this, VariableType::Vec4 };
	wywz = { this, VariableType::Vec4 };
	wyww = { this, VariableType::Vec4 };
	wzxx = { this, VariableType::Vec4 };
	wzxy = { this, VariableType::Vec4 };
	wzxz = { this, VariableType::Vec4 };
	wzxw = { this, VariableType::Vec4 };
	wzyx = { this, VariableType::Vec4 };
	wzyy = { this, VariableType::Vec4 };
	wzyz = { this, VariableType::Vec4 };
	wzyw = { this, VariableType::Vec4 };
	wzzx = { this, VariableType::Vec4 };
	wzzy = { this, VariableType::Vec4 };
	wzzz = { this, VariableType::Vec4 };
	wzzw = { this, VariableType::Vec4 };
	wzwx = { this, VariableType::Vec4 };
	wzwy = { this, VariableType::Vec4 };
	wzwz = { this, VariableType::Vec4 };
	wzww = { this, VariableType::Vec4 };
	wwxx = { this, VariableType::Vec4 };
	wwxy = { this, VariableType::Vec4 };
	wwxz = { this, VariableType::Vec4 };
	wwxw = { this, VariableType::Vec4 };
	wwyx = { this, VariableType::Vec4 };
	wwyy = { this, VariableType::Vec4 };
	wwyz = { this, VariableType::Vec4 };
	wwyw = { this, VariableType::Vec4 };
	wwzx = { this, VariableType::Vec4 };
	wwzy = { this, VariableType::Vec4 };
	wwzz = { this, VariableType::Vec4 };
	wwzw = { this, VariableType::Vec4 };
	wwwx = { this, VariableType::Vec4 };
	wwwy = { this, VariableType::Vec4 };
	wwwz = { this, VariableType::Vec4 };
	wwww = { this, VariableType::Vec4 };

	r    = { this, VariableType::Float };
	g    = { this, VariableType::Float };
	b    = { this, VariableType::Float };
	a    = { this, VariableType::Float };

	rr   = { this, VariableType::Vec2 };
	rg   = { this, VariableType::Vec2 };
	rb   = { this, VariableType::Vec2 };
	ra   = { this, VariableType::Vec2 };
	gr   = { this, VariableType::Vec2 };
	gg   = { this, VariableType::Vec2 };
	gb   = { this, VariableType::Vec2 };
	ga   = { this, VariableType::Vec2 };
	br   = { this, VariableType::Vec2 };
	bg   = { this, VariableType::Vec2 };
	bb   = { this, VariableType::Vec2 };
	ba   = { this, VariableType::Vec2 };
	ar   = { this, VariableType::Vec2 };
	ag   = { this, VariableType::Vec2 };
	ab   = { this, VariableType::Vec2 };
	aa   = { this, VariableType::Vec2 };

	rrr  = { this, VariableType::Vec3 };
	rrg  = { this, VariableType::Vec3 };
	rrb  = { this, VariableType::Vec3 };
	rra  = { this, VariableType::Vec3 };
	rgr  = { this, VariableType::Vec3 };
	rgg  = { this, VariableType::Vec3 };
	rgb  = { this, VariableType::Vec3 };
	rga  = { this, VariableType::Vec3 };
	rbr  = { this, VariableType::Vec3 };
	rbg  = { this, VariableType::Vec3 };
	rbb  = { this, VariableType::Vec3 };
	rba  = { this, VariableType::Vec3 };
	rar  = { this, VariableType::Vec3 };
	rag  = { this, VariableType::Vec3 };
	rab  = { this, VariableType::Vec3 };
	raa  = { this, VariableType::Vec3 };
	grr  = { this, VariableType::Vec3 };
	grg  = { this, VariableType::Vec3 };
	grb  = { this, VariableType::Vec3 };
	gra  = { this, VariableType::Vec3 };
	ggr  = { this, VariableType::Vec3 };
	ggg  = { this, VariableType::Vec3 };
	ggb  = { this, VariableType::Vec3 };
	gga  = { this, VariableType::Vec3 };
	gbr  = { this, VariableType::Vec3 };
	gbg  = { this, VariableType::Vec3 };
	gbb  = { this, VariableType::Vec3 };
	gba  = { this, VariableType::Vec3 };
	gar  = { this, VariableType::Vec3 };
	gag  = { this, VariableType::Vec3 };
	gab  = { this, VariableType::Vec3 };
	gaa  = { this, VariableType::Vec3 };
	brr  = { this, VariableType::Vec3 };
	brg  = { this, VariableType::Vec3 };
	brb  = { this, VariableType::Vec3 };
	bra  = { this, VariableType::Vec3 };
	bgr  = { this, VariableType::Vec3 };
	bgg  = { this, VariableType::Vec3 };
	bgb  = { this, VariableType::Vec3 };
	bga  = { this, VariableType::Vec3 };
	bbr  = { this, VariableType::Vec3 };
	bbg  = { this, VariableType::Vec3 };
	bbb  = { this, VariableType::Vec3 };
	bba  = { this, VariableType::Vec3 };
	bar  = { this, VariableType::Vec3 };
	bag  = { this, VariableType::Vec3 };
	bab  = { this, VariableType::Vec3 };
	baa  = { this, VariableType::Vec3 };
	arr  = { this, VariableType::Vec3 };
	arg  = { this, VariableType::Vec3 };
	arb  = { this, VariableType::Vec3 };
	ara  = { this, VariableType::Vec3 };
	agr  = { this, VariableType::Vec3 };
	agg  = { this, VariableType::Vec3 };
	agb  = { this, VariableType::Vec3 };
	aga  = { this, VariableType::Vec3 };
	abr  = { this, VariableType::Vec3 };
	abg  = { this, VariableType::Vec3 };
	abb  = { this, VariableType::Vec3 };
	aba  = { this, VariableType::Vec3 };
	aar  = { this, VariableType::Vec3 };
	aag  = { this, VariableType::Vec3 };
	aab  = { this, VariableType::Vec3 };
	aaa  = { this, VariableType::Vec3 };

	rrrr = { this, VariableType::Vec4 };
	rrrg = { this, VariableType::Vec4 };
	rrrb = { this, VariableType::Vec4 };
	rrra = { this, VariableType::Vec4 };
	rrgr = { this, VariableType::Vec4 };
	rrgg = { this, VariableType::Vec4 };
	rrgb = { this, VariableType::Vec4 };
	rrga = { this, VariableType::Vec4 };
	rrbr = { this, VariableType::Vec4 };
	rrbg = { this, VariableType::Vec4 };
	rrbb = { this, VariableType::Vec4 };
	rrba = { this, VariableType::Vec4 };
	rrar = { this, VariableType::Vec4 };
	rrag = { this, VariableType::Vec4 };
	rrab = { this, VariableType::Vec4 };
	rraa = { this, VariableType::Vec4 };
	rgrr = { this, VariableType::Vec4 };
	rgrg = { this, VariableType::Vec4 };
	rgrb = { this, VariableType::Vec4 };
	rgra = { this, VariableType::Vec4 };
	rggr = { this, VariableType::Vec4 };
	rggg = { this, VariableType::Vec4 };
	rggb = { this, VariableType::Vec4 };
	rgga = { this, VariableType::Vec4 };
	rgbr = { this, VariableType::Vec4 };
	rgbg = { this, VariableType::Vec4 };
	rgbb = { this, VariableType::Vec4 };
	rgba = { this, VariableType::Vec4 };
	rgar = { this, VariableType::Vec4 };
	rgag = { this, VariableType::Vec4 };
	rgab = { this, VariableType::Vec4 };
	rgaa = { this, VariableType::Vec4 };
	rbrr = { this, VariableType::Vec4 };
	rbrg = { this, VariableType::Vec4 };
	rbrb = { this, VariableType::Vec4 };
	rbra = { this, VariableType::Vec4 };
	rbgr = { this, VariableType::Vec4 };
	rbgg = { this, VariableType::Vec4 };
	rbgb = { this, VariableType::Vec4 };
	rbga = { this, VariableType::Vec4 };
	rbbr = { this, VariableType::Vec4 };
	rbbg = { this, VariableType::Vec4 };
	rbbb = { this, VariableType::Vec4 };
	rbba = { this, VariableType::Vec4 };
	rbar = { this, VariableType::Vec4 };
	rbag = { this, VariableType::Vec4 };
	rbab = { this, VariableType::Vec4 };
	rbaa = { this, VariableType::Vec4 };
	rarr = { this, VariableType::Vec4 };
	rarg = { this, VariableType::Vec4 };
	rarb = { this, VariableType::Vec4 };
	rara = { this, VariableType::Vec4 };
	ragr = { this, VariableType::Vec4 };
	ragg = { this, VariableType::Vec4 };
	ragb = { this, VariableType::Vec4 };
	raga = { this, VariableType::Vec4 };
	rabr = { this, VariableType::Vec4 };
	rabg = { this, VariableType::Vec4 };
	rabb = { this, VariableType::Vec4 };
	raba = { this, VariableType::Vec4 };
	raar = { this, VariableType::Vec4 };
	raag = { this, VariableType::Vec4 };
	raab = { this, VariableType::Vec4 };
	raaa = { this, VariableType::Vec4 };
	grrr = { this, VariableType::Vec4 };
	grrg = { this, VariableType::Vec4 };
	grrb = { this, VariableType::Vec4 };
	grra = { this, VariableType::Vec4 };
	grgr = { this, VariableType::Vec4 };
	grgg = { this, VariableType::Vec4 };
	grgb = { this, VariableType::Vec4 };
	grga = { this, VariableType::Vec4 };
	grbr = { this, VariableType::Vec4 };
	grbg = { this, VariableType::Vec4 };
	grbb = { this, VariableType::Vec4 };
	grba = { this, VariableType::Vec4 };
	grar = { this, VariableType::Vec4 };
	grag = { this, VariableType::Vec4 };
	grab = { this, VariableType::Vec4 };
	graa = { this, VariableType::Vec4 };
	ggrr = { this, VariableType::Vec4 };
	ggrg = { this, VariableType::Vec4 };
	ggrb = { this, VariableType::Vec4 };
	ggra = { this, VariableType::Vec4 };
	gggr = { this, VariableType::Vec4 };
	gggg = { this, VariableType::Vec4 };
	gggb = { this, VariableType::Vec4 };
	ggga = { this, VariableType::Vec4 };
	ggbr = { this, VariableType::Vec4 };
	ggbg = { this, VariableType::Vec4 };
	ggbb = { this, VariableType::Vec4 };
	ggba = { this, VariableType::Vec4 };
	ggar = { this, VariableType::Vec4 };
	ggag = { this, VariableType::Vec4 };
	ggab = { this, VariableType::Vec4 };
	ggaa = { this, VariableType::Vec4 };
	gbrr = { this, VariableType::Vec4 };
	gbrg = { this, VariableType::Vec4 };
	gbrb = { this, VariableType::Vec4 };
	gbra = { this, VariableType::Vec4 };
	gbgr = { this, VariableType::Vec4 };
	gbgg = { this, VariableType::Vec4 };
	gbgb = { this, VariableType::Vec4 };
	gbga = { this, VariableType::Vec4 };
	gbbr = { this, VariableType::Vec4 };
	gbbg = { this, VariableType::Vec4 };
	gbbb = { this, VariableType::Vec4 };
	gbba = { this, VariableType::Vec4 };
	gbar = { this, VariableType::Vec4 };
	gbag = { this, VariableType::Vec4 };
	gbab = { this, VariableType::Vec4 };
	gbaa = { this, VariableType::Vec4 };
	garr = { this, VariableType::Vec4 };
	garg = { this, VariableType::Vec4 };
	garb = { this, VariableType::Vec4 };
	gara = { this, VariableType::Vec4 };
	gagr = { this, VariableType::Vec4 };
	gagg = { this, VariableType::Vec4 };
	gagb = { this, VariableType::Vec4 };
	gaga = { this, VariableType::Vec4 };
	gabr = { this, VariableType::Vec4 };
	gabg = { this, VariableType::Vec4 };
	gabb = { this, VariableType::Vec4 };
	gaba = { this, VariableType::Vec4 };
	gaar = { this, VariableType::Vec4 };
	gaag = { this, VariableType::Vec4 };
	gaab = { this, VariableType::Vec4 };
	gaaa = { this, VariableType::Vec4 };
	brrr = { this, VariableType::Vec4 };
	brrg = { this, VariableType::Vec4 };
	brrb = { this, VariableType::Vec4 };
	brra = { this, VariableType::Vec4 };
	brgr = { this, VariableType::Vec4 };
	brgg = { this, VariableType::Vec4 };
	brgb = { this, VariableType::Vec4 };
	brga = { this, VariableType::Vec4 };
	brbr = { this, VariableType::Vec4 };
	brbg = { this, VariableType::Vec4 };
	brbb = { this, VariableType::Vec4 };
	brba = { this, VariableType::Vec4 };
	brar = { this, VariableType::Vec4 };
	brag = { this, VariableType::Vec4 };
	brab = { this, VariableType::Vec4 };
	braa = { this, VariableType::Vec4 };
	bgrr = { this, VariableType::Vec4 };
	bgrg = { this, VariableType::Vec4 };
	bgrb = { this, VariableType::Vec4 };
	bgra = { this, VariableType::Vec4 };
	bggr = { this, VariableType::Vec4 };
	bggg = { this, VariableType::Vec4 };
	bggb = { this, VariableType::Vec4 };
	bgga = { this, VariableType::Vec4 };
	bgbr = { this, VariableType::Vec4 };
	bgbg = { this, VariableType::Vec4 };
	bgbb = { this, VariableType::Vec4 };
	bgba = { this, VariableType::Vec4 };
	bgar = { this, VariableType::Vec4 };
	bgag = { this, VariableType::Vec4 };
	bgab = { this, VariableType::Vec4 };
	bgaa = { this, VariableType::Vec4 };
	bbrr = { this, VariableType::Vec4 };
	bbrg = { this, VariableType::Vec4 };
	bbrb = { this, VariableType::Vec4 };
	bbra = { this, VariableType::Vec4 };
	bbgr = { this, VariableType::Vec4 };
	bbgg = { this, VariableType::Vec4 };
	bbgb = { this, VariableType::Vec4 };
	bbga = { this, VariableType::Vec4 };
	bbbr = { this, VariableType::Vec4 };
	bbbg = { this, VariableType::Vec4 };
	bbbb = { this, VariableType::Vec4 };
	bbba = { this, VariableType::Vec4 };
	bbar = { this, VariableType::Vec4 };
	bbag = { this, VariableType::Vec4 };
	bbab = { this, VariableType::Vec4 };
	bbaa = { this, VariableType::Vec4 };
	barr = { this, VariableType::Vec4 };
	barg = { this, VariableType::Vec4 };
	barb = { this, VariableType::Vec4 };
	bara = { this, VariableType::Vec4 };
	bagr = { this, VariableType::Vec4 };
	bagg = { this, VariableType::Vec4 };
	bagb = { this, VariableType::Vec4 };
	baga = { this, VariableType::Vec4 };
	babr = { this, VariableType::Vec4 };
	babg = { this, VariableType::Vec4 };
	babb = { this, VariableType::Vec4 };
	baba = { this, VariableType::Vec4 };
	baar = { this, VariableType::Vec4 };
	baag = { this, VariableType::Vec4 };
	baab = { this, VariableType::Vec4 };
	baaa = { this, VariableType::Vec4 };
	arrr = { this, VariableType::Vec4 };
	arrg = { this, VariableType::Vec4 };
	arrb = { this, VariableType::Vec4 };
	arra = { this, VariableType::Vec4 };
	argr = { this, VariableType::Vec4 };
	argg = { this, VariableType::Vec4 };
	argb = { this, VariableType::Vec4 };
	arga = { this, VariableType::Vec4 };
	arbr = { this, VariableType::Vec4 };
	arbg = { this, VariableType::Vec4 };
	arbb = { this, VariableType::Vec4 };
	arba = { this, VariableType::Vec4 };
	arar = { this, VariableType::Vec4 };
	arag = { this, VariableType::Vec4 };
	arab = { this, VariableType::Vec4 };
	araa = { this, VariableType::Vec4 };
	agrr = { this, VariableType::Vec4 };
	agrg = { this, VariableType::Vec4 };
	agrb = { this, VariableType::Vec4 };
	agra = { this, VariableType::Vec4 };
	aggr = { this, VariableType::Vec4 };
	aggg = { this, VariableType::Vec4 };
	aggb = { this, VariableType::Vec4 };
	agga = { this, VariableType::Vec4 };
	agbr = { this, VariableType::Vec4 };
	agbg = { this, VariableType::Vec4 };
	agbb = { this, VariableType::Vec4 };
	agba = { this, VariableType::Vec4 };
	agar = { this, VariableType::Vec4 };
	agag = { this, VariableType::Vec4 };
	agab = { this, VariableType::Vec4 };
	agaa = { this, VariableType::Vec4 };
	abrr = { this, VariableType::Vec4 };
	abrg = { this, VariableType::Vec4 };
	abrb = { this, VariableType::Vec4 };
	abra = { this, VariableType::Vec4 };
	abgr = { this, VariableType::Vec4 };
	abgg = { this, VariableType::Vec4 };
	abgb = { this, VariableType::Vec4 };
	abga = { this, VariableType::Vec4 };
	abbr = { this, VariableType::Vec4 };
	abbg = { this, VariableType::Vec4 };
	abbb = { this, VariableType::Vec4 };
	abba = { this, VariableType::Vec4 };
	abar = { this, VariableType::Vec4 };
	abag = { this, VariableType::Vec4 };
	abab = { this, VariableType::Vec4 };
	abaa = { this, VariableType::Vec4 };
	aarr = { this, VariableType::Vec4 };
	aarg = { this, VariableType::Vec4 };
	aarb = { this, VariableType::Vec4 };
	aara = { this, VariableType::Vec4 };
	aagr = { this, VariableType::Vec4 };
	aagg = { this, VariableType::Vec4 };
	aagb = { this, VariableType::Vec4 };
	aaga = { this, VariableType::Vec4 };
	aabr = { this, VariableType::Vec4 };
	aabg = { this, VariableType::Vec4 };
	aabb = { this, VariableType::Vec4 };
	aaba = { this, VariableType::Vec4 };
	aaar = { this, VariableType::Vec4 };
	aaag = { this, VariableType::Vec4 };
	aaab = { this, VariableType::Vec4 };
	aaaa = { this, VariableType::Vec4 };
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
