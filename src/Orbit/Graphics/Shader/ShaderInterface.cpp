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

#include <map>
#include <sstream>

ORB_NAMESPACE_BEGIN

static ShaderInterface* current_shader = nullptr;

static std::string GenerateName( const std::string prefix )
{
	static std::map< std::string, uint32_t > variable_counter;

	std::ostringstream ss;
	ss << prefix;
	ss << '_';
	ss << ( variable_counter[ prefix ]++ );
	return ss.str();
}

std::string ShaderInterface::GetSource( void )
{
	if( m_source_code.empty() )
	{
		m_source_code.reserve( 4096 );
		current_shader = this;

		m_source_code.append( "\nPixelData VSMain( VertexData input )\n{\n" );
		auto vs_result = VSMain();
		m_source_code.append( "}\n" );

		LogInfo( "%s\n", m_source_code.c_str() );

		m_source_code.append( "\nfloat4 PSMain( PixelData input )\n{\n" );
		auto ps_result = PSMain();
		m_source_code.append( "}\n" );

		LogInfo( "%s\n", m_source_code.c_str() );
	}

	return m_source_code;
}

VertexLayout ShaderInterface::GetVertexLayout( void ) const
{
	return VertexLayout{ };
}

ShaderInterface::VariableDummy::VariableDummy( Variable* parent, const std::string& name )
	: m_name( parent->m_name + "." + name )
{
}

ShaderInterface::VariableDummy::operator ShaderInterface::Variable( void ) const
{
	return Variable{ m_name };
}

ShaderInterface::Variable ShaderInterface::VariableDummy::operator*=( const Variable& rhs ) const
{
	return Variable{ m_name + " *= " + rhs.m_name };
}

ShaderInterface::Variable::Variable( void )
	: m_name{ GenerateName( "local" ) }
{
	InitDummies();
}

ShaderInterface::Variable::Variable( const Variable& )
	: m_name{ GenerateName( "local" ) }
{
	InitDummies();
}

ShaderInterface::Variable::Variable( const Variable&, const Variable& )
	: m_name{ GenerateName( "local" ) }
{
	InitDummies();
}

ShaderInterface::Variable::Variable( const Variable&, const Variable&, const Variable& )
	: m_name{ GenerateName( "local" ) }
{
	InitDummies();
}

ShaderInterface::Variable::Variable( const Variable&, const Variable&, const Variable&, const Variable& )
	: m_name{ GenerateName( "local" ) }
{
	InitDummies();
}

ShaderInterface::Variable::Variable( double )
	: m_name{ GenerateName( "local" ) }
{
	InitDummies();
}

ShaderInterface::Variable::Variable( std::string_view name )
	: m_name{ name }
{
	InitDummies();
}

ShaderInterface::Variable& ShaderInterface::Variable::operator=( const Variable& rhs )
{
	current_shader->m_source_code.append( "\t" + m_name + " = " + rhs.m_name + ";\n" );

	return *this;
}

ShaderInterface::Variable ShaderInterface::Variable::operator*( const Variable& rhs ) const
{
	return Variable{ "( " + m_name + " * " + rhs.m_name + " )" };
}

ShaderInterface::Variable ShaderInterface::Variable::operator+( const Variable& rhs ) const
{
	return Variable{ "( " + m_name + " + " + rhs.m_name + " )" };
}

ShaderInterface::Variable ShaderInterface::Variable::operator-( void ) const
{
	return Variable{ "( -" + m_name + " )" };
}

ShaderInterface::Variable& ShaderInterface::Variable::operator+=( const Variable& rhs )
{
	current_shader->m_source_code.append( "( " + m_name + " += " + rhs.m_name + " )" );

	return *this;
}

ShaderInterface::Variable& ShaderInterface::Variable::operator*=( const Variable& rhs )
{
	current_shader->m_source_code.append( "( " + m_name + " *= " + rhs.m_name + " )" );

	return *this;
}

void ShaderInterface::Variable::InitDummies( void )
{
	x    = VariableDummy{ this, "x" };
	y    = VariableDummy{ this, "y" };
	z    = VariableDummy{ this, "z" };
	w    = VariableDummy{ this, "w" };

	xx   = VariableDummy{ this, "xx" };
	xy   = VariableDummy{ this, "xy" };
	xz   = VariableDummy{ this, "xz" };
	xw   = VariableDummy{ this, "xw" };
	yx   = VariableDummy{ this, "yx" };
	yy   = VariableDummy{ this, "yy" };
	yz   = VariableDummy{ this, "yz" };
	yw   = VariableDummy{ this, "yw" };
	zx   = VariableDummy{ this, "zx" };
	zy   = VariableDummy{ this, "zy" };
	zz   = VariableDummy{ this, "zz" };
	zw   = VariableDummy{ this, "zw" };
	wx   = VariableDummy{ this, "wx" };
	wy   = VariableDummy{ this, "wy" };
	wz   = VariableDummy{ this, "wz" };
	ww   = VariableDummy{ this, "ww" };

	xxx  = VariableDummy{ this, "xxx" };
	xxy  = VariableDummy{ this, "xxy" };
	xxz  = VariableDummy{ this, "xxz" };
	xxw  = VariableDummy{ this, "xxw" };
	xyx  = VariableDummy{ this, "xyx" };
	xyy  = VariableDummy{ this, "xyy" };
	xyz  = VariableDummy{ this, "xyz" };
	xyw  = VariableDummy{ this, "xyw" };
	xzx  = VariableDummy{ this, "xzx" };
	xzy  = VariableDummy{ this, "xzy" };
	xzz  = VariableDummy{ this, "xzz" };
	xzw  = VariableDummy{ this, "xzw" };
	xwx  = VariableDummy{ this, "xwx" };
	xwy  = VariableDummy{ this, "xwy" };
	xwz  = VariableDummy{ this, "xwz" };
	xww  = VariableDummy{ this, "xww" };
	yxx  = VariableDummy{ this, "yxx" };
	yxy  = VariableDummy{ this, "yxy" };
	yxz  = VariableDummy{ this, "yxz" };
	yxw  = VariableDummy{ this, "yxw" };
	yyx  = VariableDummy{ this, "yyx" };
	yyy  = VariableDummy{ this, "yyy" };
	yyz  = VariableDummy{ this, "yyz" };
	yyw  = VariableDummy{ this, "yyw" };
	yzx  = VariableDummy{ this, "yzx" };
	yzy  = VariableDummy{ this, "yzy" };
	yzz  = VariableDummy{ this, "yzz" };
	yzw  = VariableDummy{ this, "yzw" };
	ywx  = VariableDummy{ this, "ywx" };
	ywy  = VariableDummy{ this, "ywy" };
	ywz  = VariableDummy{ this, "ywz" };
	yww  = VariableDummy{ this, "yww" };
	zxx  = VariableDummy{ this, "zxx" };
	zxy  = VariableDummy{ this, "zxy" };
	zxz  = VariableDummy{ this, "zxz" };
	zxw  = VariableDummy{ this, "zxw" };
	zyx  = VariableDummy{ this, "zyx" };
	zyy  = VariableDummy{ this, "zyy" };
	zyz  = VariableDummy{ this, "zyz" };
	zyw  = VariableDummy{ this, "zyw" };
	zzx  = VariableDummy{ this, "zzx" };
	zzy  = VariableDummy{ this, "zzy" };
	zzz  = VariableDummy{ this, "zzz" };
	zzw  = VariableDummy{ this, "zzw" };
	zwx  = VariableDummy{ this, "zwx" };
	zwy  = VariableDummy{ this, "zwy" };
	zwz  = VariableDummy{ this, "zwz" };
	zww  = VariableDummy{ this, "zww" };
	wxx  = VariableDummy{ this, "wxx" };
	wxy  = VariableDummy{ this, "wxy" };
	wxz  = VariableDummy{ this, "wxz" };
	wxw  = VariableDummy{ this, "wxw" };
	wyx  = VariableDummy{ this, "wyx" };
	wyy  = VariableDummy{ this, "wyy" };
	wyz  = VariableDummy{ this, "wyz" };
	wyw  = VariableDummy{ this, "wyw" };
	wzx  = VariableDummy{ this, "wzx" };
	wzy  = VariableDummy{ this, "wzy" };
	wzz  = VariableDummy{ this, "wzz" };
	wzw  = VariableDummy{ this, "wzw" };
	wwx  = VariableDummy{ this, "wwx" };
	wwy  = VariableDummy{ this, "wwy" };
	wwz  = VariableDummy{ this, "wwz" };
	www  = VariableDummy{ this, "www" };

	xxxx = VariableDummy{ this, "xxxx" };
	xxxy = VariableDummy{ this, "xxxy" };
	xxxz = VariableDummy{ this, "xxxz" };
	xxxw = VariableDummy{ this, "xxxw" };
	xxyx = VariableDummy{ this, "xxyx" };
	xxyy = VariableDummy{ this, "xxyy" };
	xxyz = VariableDummy{ this, "xxyz" };
	xxyw = VariableDummy{ this, "xxyw" };
	xxzx = VariableDummy{ this, "xxzx" };
	xxzy = VariableDummy{ this, "xxzy" };
	xxzz = VariableDummy{ this, "xxzz" };
	xxzw = VariableDummy{ this, "xxzw" };
	xxwx = VariableDummy{ this, "xxwx" };
	xxwy = VariableDummy{ this, "xxwy" };
	xxwz = VariableDummy{ this, "xxwz" };
	xxww = VariableDummy{ this, "xxww" };
	xyxx = VariableDummy{ this, "xyxx" };
	xyxy = VariableDummy{ this, "xyxy" };
	xyxz = VariableDummy{ this, "xyxz" };
	xyxw = VariableDummy{ this, "xyxw" };
	xyyx = VariableDummy{ this, "xyyx" };
	xyyy = VariableDummy{ this, "xyyy" };
	xyyz = VariableDummy{ this, "xyyz" };
	xyyw = VariableDummy{ this, "xyyw" };
	xyzx = VariableDummy{ this, "xyzx" };
	xyzy = VariableDummy{ this, "xyzy" };
	xyzz = VariableDummy{ this, "xyzz" };
	xyzw = VariableDummy{ this, "xyzw" };
	xywx = VariableDummy{ this, "xywx" };
	xywy = VariableDummy{ this, "xywy" };
	xywz = VariableDummy{ this, "xywz" };
	xyww = VariableDummy{ this, "xyww" };
	xzxx = VariableDummy{ this, "xzxx" };
	xzxy = VariableDummy{ this, "xzxy" };
	xzxz = VariableDummy{ this, "xzxz" };
	xzxw = VariableDummy{ this, "xzxw" };
	xzyx = VariableDummy{ this, "xzyx" };
	xzyy = VariableDummy{ this, "xzyy" };
	xzyz = VariableDummy{ this, "xzyz" };
	xzyw = VariableDummy{ this, "xzyw" };
	xzzx = VariableDummy{ this, "xzzx" };
	xzzy = VariableDummy{ this, "xzzy" };
	xzzz = VariableDummy{ this, "xzzz" };
	xzzw = VariableDummy{ this, "xzzw" };
	xzwx = VariableDummy{ this, "xzwx" };
	xzwy = VariableDummy{ this, "xzwy" };
	xzwz = VariableDummy{ this, "xzwz" };
	xzww = VariableDummy{ this, "xzww" };
	xwxx = VariableDummy{ this, "xwxx" };
	xwxy = VariableDummy{ this, "xwxy" };
	xwxz = VariableDummy{ this, "xwxz" };
	xwxw = VariableDummy{ this, "xwxw" };
	xwyx = VariableDummy{ this, "xwyx" };
	xwyy = VariableDummy{ this, "xwyy" };
	xwyz = VariableDummy{ this, "xwyz" };
	xwyw = VariableDummy{ this, "xwyw" };
	xwzx = VariableDummy{ this, "xwzx" };
	xwzy = VariableDummy{ this, "xwzy" };
	xwzz = VariableDummy{ this, "xwzz" };
	xwzw = VariableDummy{ this, "xwzw" };
	xwwx = VariableDummy{ this, "xwwx" };
	xwwy = VariableDummy{ this, "xwwy" };
	xwwz = VariableDummy{ this, "xwwz" };
	xwww = VariableDummy{ this, "xwww" };
	yxxx = VariableDummy{ this, "yxxx" };
	yxxy = VariableDummy{ this, "yxxy" };
	yxxz = VariableDummy{ this, "yxxz" };
	yxxw = VariableDummy{ this, "yxxw" };
	yxyx = VariableDummy{ this, "yxyx" };
	yxyy = VariableDummy{ this, "yxyy" };
	yxyz = VariableDummy{ this, "yxyz" };
	yxyw = VariableDummy{ this, "yxyw" };
	yxzx = VariableDummy{ this, "yxzx" };
	yxzy = VariableDummy{ this, "yxzy" };
	yxzz = VariableDummy{ this, "yxzz" };
	yxzw = VariableDummy{ this, "yxzw" };
	yxwx = VariableDummy{ this, "yxwx" };
	yxwy = VariableDummy{ this, "yxwy" };
	yxwz = VariableDummy{ this, "yxwz" };
	yxww = VariableDummy{ this, "yxww" };
	yyxx = VariableDummy{ this, "yyxx" };
	yyxy = VariableDummy{ this, "yyxy" };
	yyxz = VariableDummy{ this, "yyxz" };
	yyxw = VariableDummy{ this, "yyxw" };
	yyyx = VariableDummy{ this, "yyyx" };
	yyyy = VariableDummy{ this, "yyyy" };
	yyyz = VariableDummy{ this, "yyyz" };
	yyyw = VariableDummy{ this, "yyyw" };
	yyzx = VariableDummy{ this, "yyzx" };
	yyzy = VariableDummy{ this, "yyzy" };
	yyzz = VariableDummy{ this, "yyzz" };
	yyzw = VariableDummy{ this, "yyzw" };
	yywx = VariableDummy{ this, "yywx" };
	yywy = VariableDummy{ this, "yywy" };
	yywz = VariableDummy{ this, "yywz" };
	yyww = VariableDummy{ this, "yyww" };
	yzxx = VariableDummy{ this, "yzxx" };
	yzxy = VariableDummy{ this, "yzxy" };
	yzxz = VariableDummy{ this, "yzxz" };
	yzxw = VariableDummy{ this, "yzxw" };
	yzyx = VariableDummy{ this, "yzyx" };
	yzyy = VariableDummy{ this, "yzyy" };
	yzyz = VariableDummy{ this, "yzyz" };
	yzyw = VariableDummy{ this, "yzyw" };
	yzzx = VariableDummy{ this, "yzzx" };
	yzzy = VariableDummy{ this, "yzzy" };
	yzzz = VariableDummy{ this, "yzzz" };
	yzzw = VariableDummy{ this, "yzzw" };
	yzwx = VariableDummy{ this, "yzwx" };
	yzwy = VariableDummy{ this, "yzwy" };
	yzwz = VariableDummy{ this, "yzwz" };
	yzww = VariableDummy{ this, "yzww" };
	ywxx = VariableDummy{ this, "ywxx" };
	ywxy = VariableDummy{ this, "ywxy" };
	ywxz = VariableDummy{ this, "ywxz" };
	ywxw = VariableDummy{ this, "ywxw" };
	ywyx = VariableDummy{ this, "ywyx" };
	ywyy = VariableDummy{ this, "ywyy" };
	ywyz = VariableDummy{ this, "ywyz" };
	ywyw = VariableDummy{ this, "ywyw" };
	ywzx = VariableDummy{ this, "ywzx" };
	ywzy = VariableDummy{ this, "ywzy" };
	ywzz = VariableDummy{ this, "ywzz" };
	ywzw = VariableDummy{ this, "ywzw" };
	ywwx = VariableDummy{ this, "ywwx" };
	ywwy = VariableDummy{ this, "ywwy" };
	ywwz = VariableDummy{ this, "ywwz" };
	ywww = VariableDummy{ this, "ywww" };
	zxxx = VariableDummy{ this, "zxxx" };
	zxxy = VariableDummy{ this, "zxxy" };
	zxxz = VariableDummy{ this, "zxxz" };
	zxxw = VariableDummy{ this, "zxxw" };
	zxyx = VariableDummy{ this, "zxyx" };
	zxyy = VariableDummy{ this, "zxyy" };
	zxyz = VariableDummy{ this, "zxyz" };
	zxyw = VariableDummy{ this, "zxyw" };
	zxzx = VariableDummy{ this, "zxzx" };
	zxzy = VariableDummy{ this, "zxzy" };
	zxzz = VariableDummy{ this, "zxzz" };
	zxzw = VariableDummy{ this, "zxzw" };
	zxwx = VariableDummy{ this, "zxwx" };
	zxwy = VariableDummy{ this, "zxwy" };
	zxwz = VariableDummy{ this, "zxwz" };
	zxww = VariableDummy{ this, "zxww" };
	zyxx = VariableDummy{ this, "zyxx" };
	zyxy = VariableDummy{ this, "zyxy" };
	zyxz = VariableDummy{ this, "zyxz" };
	zyxw = VariableDummy{ this, "zyxw" };
	zyyx = VariableDummy{ this, "zyyx" };
	zyyy = VariableDummy{ this, "zyyy" };
	zyyz = VariableDummy{ this, "zyyz" };
	zyyw = VariableDummy{ this, "zyyw" };
	zyzx = VariableDummy{ this, "zyzx" };
	zyzy = VariableDummy{ this, "zyzy" };
	zyzz = VariableDummy{ this, "zyzz" };
	zyzw = VariableDummy{ this, "zyzw" };
	zywx = VariableDummy{ this, "zywx" };
	zywy = VariableDummy{ this, "zywy" };
	zywz = VariableDummy{ this, "zywz" };
	zyww = VariableDummy{ this, "zyww" };
	zzxx = VariableDummy{ this, "zzxx" };
	zzxy = VariableDummy{ this, "zzxy" };
	zzxz = VariableDummy{ this, "zzxz" };
	zzxw = VariableDummy{ this, "zzxw" };
	zzyx = VariableDummy{ this, "zzyx" };
	zzyy = VariableDummy{ this, "zzyy" };
	zzyz = VariableDummy{ this, "zzyz" };
	zzyw = VariableDummy{ this, "zzyw" };
	zzzx = VariableDummy{ this, "zzzx" };
	zzzy = VariableDummy{ this, "zzzy" };
	zzzz = VariableDummy{ this, "zzzz" };
	zzzw = VariableDummy{ this, "zzzw" };
	zzwx = VariableDummy{ this, "zzwx" };
	zzwy = VariableDummy{ this, "zzwy" };
	zzwz = VariableDummy{ this, "zzwz" };
	zzww = VariableDummy{ this, "zzww" };
	zwxx = VariableDummy{ this, "zwxx" };
	zwxy = VariableDummy{ this, "zwxy" };
	zwxz = VariableDummy{ this, "zwxz" };
	zwxw = VariableDummy{ this, "zwxw" };
	zwyx = VariableDummy{ this, "zwyx" };
	zwyy = VariableDummy{ this, "zwyy" };
	zwyz = VariableDummy{ this, "zwyz" };
	zwyw = VariableDummy{ this, "zwyw" };
	zwzx = VariableDummy{ this, "zwzx" };
	zwzy = VariableDummy{ this, "zwzy" };
	zwzz = VariableDummy{ this, "zwzz" };
	zwzw = VariableDummy{ this, "zwzw" };
	zwwx = VariableDummy{ this, "zwwx" };
	zwwy = VariableDummy{ this, "zwwy" };
	zwwz = VariableDummy{ this, "zwwz" };
	zwww = VariableDummy{ this, "zwww" };
	wxxx = VariableDummy{ this, "wxxx" };
	wxxy = VariableDummy{ this, "wxxy" };
	wxxz = VariableDummy{ this, "wxxz" };
	wxxw = VariableDummy{ this, "wxxw" };
	wxyx = VariableDummy{ this, "wxyx" };
	wxyy = VariableDummy{ this, "wxyy" };
	wxyz = VariableDummy{ this, "wxyz" };
	wxyw = VariableDummy{ this, "wxyw" };
	wxzx = VariableDummy{ this, "wxzx" };
	wxzy = VariableDummy{ this, "wxzy" };
	wxzz = VariableDummy{ this, "wxzz" };
	wxzw = VariableDummy{ this, "wxzw" };
	wxwx = VariableDummy{ this, "wxwx" };
	wxwy = VariableDummy{ this, "wxwy" };
	wxwz = VariableDummy{ this, "wxwz" };
	wxww = VariableDummy{ this, "wxww" };
	wyxx = VariableDummy{ this, "wyxx" };
	wyxy = VariableDummy{ this, "wyxy" };
	wyxz = VariableDummy{ this, "wyxz" };
	wyxw = VariableDummy{ this, "wyxw" };
	wyyx = VariableDummy{ this, "wyyx" };
	wyyy = VariableDummy{ this, "wyyy" };
	wyyz = VariableDummy{ this, "wyyz" };
	wyyw = VariableDummy{ this, "wyyw" };
	wyzx = VariableDummy{ this, "wyzx" };
	wyzy = VariableDummy{ this, "wyzy" };
	wyzz = VariableDummy{ this, "wyzz" };
	wyzw = VariableDummy{ this, "wyzw" };
	wywx = VariableDummy{ this, "wywx" };
	wywy = VariableDummy{ this, "wywy" };
	wywz = VariableDummy{ this, "wywz" };
	wyww = VariableDummy{ this, "wyww" };
	wzxx = VariableDummy{ this, "wzxx" };
	wzxy = VariableDummy{ this, "wzxy" };
	wzxz = VariableDummy{ this, "wzxz" };
	wzxw = VariableDummy{ this, "wzxw" };
	wzyx = VariableDummy{ this, "wzyx" };
	wzyy = VariableDummy{ this, "wzyy" };
	wzyz = VariableDummy{ this, "wzyz" };
	wzyw = VariableDummy{ this, "wzyw" };
	wzzx = VariableDummy{ this, "wzzx" };
	wzzy = VariableDummy{ this, "wzzy" };
	wzzz = VariableDummy{ this, "wzzz" };
	wzzw = VariableDummy{ this, "wzzw" };
	wzwx = VariableDummy{ this, "wzwx" };
	wzwy = VariableDummy{ this, "wzwy" };
	wzwz = VariableDummy{ this, "wzwz" };
	wzww = VariableDummy{ this, "wzww" };
	wwxx = VariableDummy{ this, "wwxx" };
	wwxy = VariableDummy{ this, "wwxy" };
	wwxz = VariableDummy{ this, "wwxz" };
	wwxw = VariableDummy{ this, "wwxw" };
	wwyx = VariableDummy{ this, "wwyx" };
	wwyy = VariableDummy{ this, "wwyy" };
	wwyz = VariableDummy{ this, "wwyz" };
	wwyw = VariableDummy{ this, "wwyw" };
	wwzx = VariableDummy{ this, "wwzx" };
	wwzy = VariableDummy{ this, "wwzy" };
	wwzz = VariableDummy{ this, "wwzz" };
	wwzw = VariableDummy{ this, "wwzw" };
	wwwx = VariableDummy{ this, "wwwx" };
	wwwy = VariableDummy{ this, "wwwy" };
	wwwz = VariableDummy{ this, "wwwz" };
	wwww = VariableDummy{ this, "wwww" };

	r    = VariableDummy{ this, "r" };
	g    = VariableDummy{ this, "g" };
	b    = VariableDummy{ this, "b" };
	a    = VariableDummy{ this, "a" };

	rr   = VariableDummy{ this, "rr" };
	rg   = VariableDummy{ this, "rg" };
	rb   = VariableDummy{ this, "rb" };
	ra   = VariableDummy{ this, "ra" };
	gr   = VariableDummy{ this, "gr" };
	gg   = VariableDummy{ this, "gg" };
	gb   = VariableDummy{ this, "gb" };
	ga   = VariableDummy{ this, "ga" };
	br   = VariableDummy{ this, "br" };
	bg   = VariableDummy{ this, "bg" };
	bb   = VariableDummy{ this, "bb" };
	ba   = VariableDummy{ this, "ba" };
	ar   = VariableDummy{ this, "ar" };
	ag   = VariableDummy{ this, "ag" };
	ab   = VariableDummy{ this, "ab" };
	aa   = VariableDummy{ this, "aa" };

	rrr  = VariableDummy{ this, "rrr" };
	rrg  = VariableDummy{ this, "rrg" };
	rrb  = VariableDummy{ this, "rrb" };
	rra  = VariableDummy{ this, "rra" };
	rgr  = VariableDummy{ this, "rgr" };
	rgg  = VariableDummy{ this, "rgg" };
	rgb  = VariableDummy{ this, "rgb" };
	rga  = VariableDummy{ this, "rga" };
	rbr  = VariableDummy{ this, "rbr" };
	rbg  = VariableDummy{ this, "rbg" };
	rbb  = VariableDummy{ this, "rbb" };
	rba  = VariableDummy{ this, "rba" };
	rar  = VariableDummy{ this, "rar" };
	rag  = VariableDummy{ this, "rag" };
	rab  = VariableDummy{ this, "rab" };
	raa  = VariableDummy{ this, "raa" };
	grr  = VariableDummy{ this, "grr" };
	grg  = VariableDummy{ this, "grg" };
	grb  = VariableDummy{ this, "grb" };
	gra  = VariableDummy{ this, "gra" };
	ggr  = VariableDummy{ this, "ggr" };
	ggg  = VariableDummy{ this, "ggg" };
	ggb  = VariableDummy{ this, "ggb" };
	gga  = VariableDummy{ this, "gga" };
	gbr  = VariableDummy{ this, "gbr" };
	gbg  = VariableDummy{ this, "gbg" };
	gbb  = VariableDummy{ this, "gbb" };
	gba  = VariableDummy{ this, "gba" };
	gar  = VariableDummy{ this, "gar" };
	gag  = VariableDummy{ this, "gag" };
	gab  = VariableDummy{ this, "gab" };
	gaa  = VariableDummy{ this, "gaa" };
	brr  = VariableDummy{ this, "brr" };
	brg  = VariableDummy{ this, "brg" };
	brb  = VariableDummy{ this, "brb" };
	bra  = VariableDummy{ this, "bra" };
	bgr  = VariableDummy{ this, "bgr" };
	bgg  = VariableDummy{ this, "bgg" };
	bgb  = VariableDummy{ this, "bgb" };
	bga  = VariableDummy{ this, "bga" };
	bbr  = VariableDummy{ this, "bbr" };
	bbg  = VariableDummy{ this, "bbg" };
	bbb  = VariableDummy{ this, "bbb" };
	bba  = VariableDummy{ this, "bba" };
	bar  = VariableDummy{ this, "bar" };
	bag  = VariableDummy{ this, "bag" };
	bab  = VariableDummy{ this, "bab" };
	baa  = VariableDummy{ this, "baa" };
	arr  = VariableDummy{ this, "arr" };
	arg  = VariableDummy{ this, "arg" };
	arb  = VariableDummy{ this, "arb" };
	ara  = VariableDummy{ this, "ara" };
	agr  = VariableDummy{ this, "agr" };
	agg  = VariableDummy{ this, "agg" };
	agb  = VariableDummy{ this, "agb" };
	aga  = VariableDummy{ this, "aga" };
	abr  = VariableDummy{ this, "abr" };
	abg  = VariableDummy{ this, "abg" };
	abb  = VariableDummy{ this, "abb" };
	aba  = VariableDummy{ this, "aba" };
	aar  = VariableDummy{ this, "aar" };
	aag  = VariableDummy{ this, "aag" };
	aab  = VariableDummy{ this, "aab" };
	aaa  = VariableDummy{ this, "aaa" };

	rrrr = VariableDummy{ this, "rrrr" };
	rrrg = VariableDummy{ this, "rrrg" };
	rrrb = VariableDummy{ this, "rrrb" };
	rrra = VariableDummy{ this, "rrra" };
	rrgr = VariableDummy{ this, "rrgr" };
	rrgg = VariableDummy{ this, "rrgg" };
	rrgb = VariableDummy{ this, "rrgb" };
	rrga = VariableDummy{ this, "rrga" };
	rrbr = VariableDummy{ this, "rrbr" };
	rrbg = VariableDummy{ this, "rrbg" };
	rrbb = VariableDummy{ this, "rrbb" };
	rrba = VariableDummy{ this, "rrba" };
	rrar = VariableDummy{ this, "rrar" };
	rrag = VariableDummy{ this, "rrag" };
	rrab = VariableDummy{ this, "rrab" };
	rraa = VariableDummy{ this, "rraa" };
	rgrr = VariableDummy{ this, "rgrr" };
	rgrg = VariableDummy{ this, "rgrg" };
	rgrb = VariableDummy{ this, "rgrb" };
	rgra = VariableDummy{ this, "rgra" };
	rggr = VariableDummy{ this, "rggr" };
	rggg = VariableDummy{ this, "rggg" };
	rggb = VariableDummy{ this, "rggb" };
	rgga = VariableDummy{ this, "rgga" };
	rgbr = VariableDummy{ this, "rgbr" };
	rgbg = VariableDummy{ this, "rgbg" };
	rgbb = VariableDummy{ this, "rgbb" };
	rgba = VariableDummy{ this, "rgba" };
	rgar = VariableDummy{ this, "rgar" };
	rgag = VariableDummy{ this, "rgag" };
	rgab = VariableDummy{ this, "rgab" };
	rgaa = VariableDummy{ this, "rgaa" };
	rbrr = VariableDummy{ this, "rbrr" };
	rbrg = VariableDummy{ this, "rbrg" };
	rbrb = VariableDummy{ this, "rbrb" };
	rbra = VariableDummy{ this, "rbra" };
	rbgr = VariableDummy{ this, "rbgr" };
	rbgg = VariableDummy{ this, "rbgg" };
	rbgb = VariableDummy{ this, "rbgb" };
	rbga = VariableDummy{ this, "rbga" };
	rbbr = VariableDummy{ this, "rbbr" };
	rbbg = VariableDummy{ this, "rbbg" };
	rbbb = VariableDummy{ this, "rbbb" };
	rbba = VariableDummy{ this, "rbba" };
	rbar = VariableDummy{ this, "rbar" };
	rbag = VariableDummy{ this, "rbag" };
	rbab = VariableDummy{ this, "rbab" };
	rbaa = VariableDummy{ this, "rbaa" };
	rarr = VariableDummy{ this, "rarr" };
	rarg = VariableDummy{ this, "rarg" };
	rarb = VariableDummy{ this, "rarb" };
	rara = VariableDummy{ this, "rara" };
	ragr = VariableDummy{ this, "ragr" };
	ragg = VariableDummy{ this, "ragg" };
	ragb = VariableDummy{ this, "ragb" };
	raga = VariableDummy{ this, "raga" };
	rabr = VariableDummy{ this, "rabr" };
	rabg = VariableDummy{ this, "rabg" };
	rabb = VariableDummy{ this, "rabb" };
	raba = VariableDummy{ this, "raba" };
	raar = VariableDummy{ this, "raar" };
	raag = VariableDummy{ this, "raag" };
	raab = VariableDummy{ this, "raab" };
	raaa = VariableDummy{ this, "raaa" };
	grrr = VariableDummy{ this, "grrr" };
	grrg = VariableDummy{ this, "grrg" };
	grrb = VariableDummy{ this, "grrb" };
	grra = VariableDummy{ this, "grra" };
	grgr = VariableDummy{ this, "grgr" };
	grgg = VariableDummy{ this, "grgg" };
	grgb = VariableDummy{ this, "grgb" };
	grga = VariableDummy{ this, "grga" };
	grbr = VariableDummy{ this, "grbr" };
	grbg = VariableDummy{ this, "grbg" };
	grbb = VariableDummy{ this, "grbb" };
	grba = VariableDummy{ this, "grba" };
	grar = VariableDummy{ this, "grar" };
	grag = VariableDummy{ this, "grag" };
	grab = VariableDummy{ this, "grab" };
	graa = VariableDummy{ this, "graa" };
	ggrr = VariableDummy{ this, "ggrr" };
	ggrg = VariableDummy{ this, "ggrg" };
	ggrb = VariableDummy{ this, "ggrb" };
	ggra = VariableDummy{ this, "ggra" };
	gggr = VariableDummy{ this, "gggr" };
	gggg = VariableDummy{ this, "gggg" };
	gggb = VariableDummy{ this, "gggb" };
	ggga = VariableDummy{ this, "ggga" };
	ggbr = VariableDummy{ this, "ggbr" };
	ggbg = VariableDummy{ this, "ggbg" };
	ggbb = VariableDummy{ this, "ggbb" };
	ggba = VariableDummy{ this, "ggba" };
	ggar = VariableDummy{ this, "ggar" };
	ggag = VariableDummy{ this, "ggag" };
	ggab = VariableDummy{ this, "ggab" };
	ggaa = VariableDummy{ this, "ggaa" };
	gbrr = VariableDummy{ this, "gbrr" };
	gbrg = VariableDummy{ this, "gbrg" };
	gbrb = VariableDummy{ this, "gbrb" };
	gbra = VariableDummy{ this, "gbra" };
	gbgr = VariableDummy{ this, "gbgr" };
	gbgg = VariableDummy{ this, "gbgg" };
	gbgb = VariableDummy{ this, "gbgb" };
	gbga = VariableDummy{ this, "gbga" };
	gbbr = VariableDummy{ this, "gbbr" };
	gbbg = VariableDummy{ this, "gbbg" };
	gbbb = VariableDummy{ this, "gbbb" };
	gbba = VariableDummy{ this, "gbba" };
	gbar = VariableDummy{ this, "gbar" };
	gbag = VariableDummy{ this, "gbag" };
	gbab = VariableDummy{ this, "gbab" };
	gbaa = VariableDummy{ this, "gbaa" };
	garr = VariableDummy{ this, "garr" };
	garg = VariableDummy{ this, "garg" };
	garb = VariableDummy{ this, "garb" };
	gara = VariableDummy{ this, "gara" };
	gagr = VariableDummy{ this, "gagr" };
	gagg = VariableDummy{ this, "gagg" };
	gagb = VariableDummy{ this, "gagb" };
	gaga = VariableDummy{ this, "gaga" };
	gabr = VariableDummy{ this, "gabr" };
	gabg = VariableDummy{ this, "gabg" };
	gabb = VariableDummy{ this, "gabb" };
	gaba = VariableDummy{ this, "gaba" };
	gaar = VariableDummy{ this, "gaar" };
	gaag = VariableDummy{ this, "gaag" };
	gaab = VariableDummy{ this, "gaab" };
	gaaa = VariableDummy{ this, "gaaa" };
	brrr = VariableDummy{ this, "brrr" };
	brrg = VariableDummy{ this, "brrg" };
	brrb = VariableDummy{ this, "brrb" };
	brra = VariableDummy{ this, "brra" };
	brgr = VariableDummy{ this, "brgr" };
	brgg = VariableDummy{ this, "brgg" };
	brgb = VariableDummy{ this, "brgb" };
	brga = VariableDummy{ this, "brga" };
	brbr = VariableDummy{ this, "brbr" };
	brbg = VariableDummy{ this, "brbg" };
	brbb = VariableDummy{ this, "brbb" };
	brba = VariableDummy{ this, "brba" };
	brar = VariableDummy{ this, "brar" };
	brag = VariableDummy{ this, "brag" };
	brab = VariableDummy{ this, "brab" };
	braa = VariableDummy{ this, "braa" };
	bgrr = VariableDummy{ this, "bgrr" };
	bgrg = VariableDummy{ this, "bgrg" };
	bgrb = VariableDummy{ this, "bgrb" };
	bgra = VariableDummy{ this, "bgra" };
	bggr = VariableDummy{ this, "bggr" };
	bggg = VariableDummy{ this, "bggg" };
	bggb = VariableDummy{ this, "bggb" };
	bgga = VariableDummy{ this, "bgga" };
	bgbr = VariableDummy{ this, "bgbr" };
	bgbg = VariableDummy{ this, "bgbg" };
	bgbb = VariableDummy{ this, "bgbb" };
	bgba = VariableDummy{ this, "bgba" };
	bgar = VariableDummy{ this, "bgar" };
	bgag = VariableDummy{ this, "bgag" };
	bgab = VariableDummy{ this, "bgab" };
	bgaa = VariableDummy{ this, "bgaa" };
	bbrr = VariableDummy{ this, "bbrr" };
	bbrg = VariableDummy{ this, "bbrg" };
	bbrb = VariableDummy{ this, "bbrb" };
	bbra = VariableDummy{ this, "bbra" };
	bbgr = VariableDummy{ this, "bbgr" };
	bbgg = VariableDummy{ this, "bbgg" };
	bbgb = VariableDummy{ this, "bbgb" };
	bbga = VariableDummy{ this, "bbga" };
	bbbr = VariableDummy{ this, "bbbr" };
	bbbg = VariableDummy{ this, "bbbg" };
	bbbb = VariableDummy{ this, "bbbb" };
	bbba = VariableDummy{ this, "bbba" };
	bbar = VariableDummy{ this, "bbar" };
	bbag = VariableDummy{ this, "bbag" };
	bbab = VariableDummy{ this, "bbab" };
	bbaa = VariableDummy{ this, "bbaa" };
	barr = VariableDummy{ this, "barr" };
	barg = VariableDummy{ this, "barg" };
	barb = VariableDummy{ this, "barb" };
	bara = VariableDummy{ this, "bara" };
	bagr = VariableDummy{ this, "bagr" };
	bagg = VariableDummy{ this, "bagg" };
	bagb = VariableDummy{ this, "bagb" };
	baga = VariableDummy{ this, "baga" };
	babr = VariableDummy{ this, "babr" };
	babg = VariableDummy{ this, "babg" };
	babb = VariableDummy{ this, "babb" };
	baba = VariableDummy{ this, "baba" };
	baar = VariableDummy{ this, "baar" };
	baag = VariableDummy{ this, "baag" };
	baab = VariableDummy{ this, "baab" };
	baaa = VariableDummy{ this, "baaa" };
	arrr = VariableDummy{ this, "arrr" };
	arrg = VariableDummy{ this, "arrg" };
	arrb = VariableDummy{ this, "arrb" };
	arra = VariableDummy{ this, "arra" };
	argr = VariableDummy{ this, "argr" };
	argg = VariableDummy{ this, "argg" };
	argb = VariableDummy{ this, "argb" };
	arga = VariableDummy{ this, "arga" };
	arbr = VariableDummy{ this, "arbr" };
	arbg = VariableDummy{ this, "arbg" };
	arbb = VariableDummy{ this, "arbb" };
	arba = VariableDummy{ this, "arba" };
	arar = VariableDummy{ this, "arar" };
	arag = VariableDummy{ this, "arag" };
	arab = VariableDummy{ this, "arab" };
	araa = VariableDummy{ this, "araa" };
	agrr = VariableDummy{ this, "agrr" };
	agrg = VariableDummy{ this, "agrg" };
	agrb = VariableDummy{ this, "agrb" };
	agra = VariableDummy{ this, "agra" };
	aggr = VariableDummy{ this, "aggr" };
	aggg = VariableDummy{ this, "aggg" };
	aggb = VariableDummy{ this, "aggb" };
	agga = VariableDummy{ this, "agga" };
	agbr = VariableDummy{ this, "agbr" };
	agbg = VariableDummy{ this, "agbg" };
	agbb = VariableDummy{ this, "agbb" };
	agba = VariableDummy{ this, "agba" };
	agar = VariableDummy{ this, "agar" };
	agag = VariableDummy{ this, "agag" };
	agab = VariableDummy{ this, "agab" };
	agaa = VariableDummy{ this, "agaa" };
	abrr = VariableDummy{ this, "abrr" };
	abrg = VariableDummy{ this, "abrg" };
	abrb = VariableDummy{ this, "abrb" };
	abra = VariableDummy{ this, "abra" };
	abgr = VariableDummy{ this, "abgr" };
	abgg = VariableDummy{ this, "abgg" };
	abgb = VariableDummy{ this, "abgb" };
	abga = VariableDummy{ this, "abga" };
	abbr = VariableDummy{ this, "abbr" };
	abbg = VariableDummy{ this, "abbg" };
	abbb = VariableDummy{ this, "abbb" };
	abba = VariableDummy{ this, "abba" };
	abar = VariableDummy{ this, "abar" };
	abag = VariableDummy{ this, "abag" };
	abab = VariableDummy{ this, "abab" };
	abaa = VariableDummy{ this, "abaa" };
	aarr = VariableDummy{ this, "aarr" };
	aarg = VariableDummy{ this, "aarg" };
	aarb = VariableDummy{ this, "aarb" };
	aara = VariableDummy{ this, "aara" };
	aagr = VariableDummy{ this, "aagr" };
	aagg = VariableDummy{ this, "aagg" };
	aagb = VariableDummy{ this, "aagb" };
	aaga = VariableDummy{ this, "aaga" };
	aabr = VariableDummy{ this, "aabr" };
	aabg = VariableDummy{ this, "aabg" };
	aabb = VariableDummy{ this, "aabb" };
	aaba = VariableDummy{ this, "aaba" };
	aaar = VariableDummy{ this, "aaar" };
	aaag = VariableDummy{ this, "aaag" };
	aaab = VariableDummy{ this, "aaab" };
	aaaa = VariableDummy{ this, "aaaa" };
}

ShaderInterface::Sampler::Sampler( void )
	: Variable( GenerateName( "sampler" ) )
{
}

ShaderInterface::Varying::Varying( void )
	: Variable( GenerateName( "varying" ) )
{
}

ShaderInterface::Attribute::Attribute( void )
	: Variable( GenerateName( "attribute" ) )
{
}

ShaderInterface::Uniform::Uniform( void )
	: Variable( GenerateName( "uniform" ) )
{
}

ShaderInterface::Variable ShaderInterface::Transpose( const Variable& rhs )
{
	return Variable{ "transpose( " + rhs.m_name + " )" };
}

ShaderInterface::Variable ShaderInterface::Sample( const Variable& sampler, const Variable& texcoord )
{
	return Variable{ sampler.m_name + ".Sample( default_sampler_state, " + texcoord.m_name + " )" };
}

ShaderInterface::Variable ShaderInterface::Dot( const Variable& vec1, const Variable& vec2 )
{
	return Variable{ "dot( " + vec1.m_name + ", " + vec2.m_name + " )" };
}

ORB_NAMESPACE_END
