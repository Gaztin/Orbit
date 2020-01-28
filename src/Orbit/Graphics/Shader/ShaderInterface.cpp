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
	x    = { this };
	y    = { this };
	z    = { this };
	w    = { this };

	xx   = { this };
	xy   = { this };
	xz   = { this };
	xw   = { this };
	yx   = { this };
	yy   = { this };
	yz   = { this };
	yw   = { this };
	zx   = { this };
	zy   = { this };
	zz   = { this };
	zw   = { this };
	wx   = { this };
	wy   = { this };
	wz   = { this };
	ww   = { this };

	xxx  = { this };
	xxy  = { this };
	xxz  = { this };
	xxw  = { this };
	xyx  = { this };
	xyy  = { this };
	xyz  = { this };
	xyw  = { this };
	xzx  = { this };
	xzy  = { this };
	xzz  = { this };
	xzw  = { this };
	xwx  = { this };
	xwy  = { this };
	xwz  = { this };
	xww  = { this };
	yxx  = { this };
	yxy  = { this };
	yxz  = { this };
	yxw  = { this };
	yyx  = { this };
	yyy  = { this };
	yyz  = { this };
	yyw  = { this };
	yzx  = { this };
	yzy  = { this };
	yzz  = { this };
	yzw  = { this };
	ywx  = { this };
	ywy  = { this };
	ywz  = { this };
	yww  = { this };
	zxx  = { this };
	zxy  = { this };
	zxz  = { this };
	zxw  = { this };
	zyx  = { this };
	zyy  = { this };
	zyz  = { this };
	zyw  = { this };
	zzx  = { this };
	zzy  = { this };
	zzz  = { this };
	zzw  = { this };
	zwx  = { this };
	zwy  = { this };
	zwz  = { this };
	zww  = { this };
	wxx  = { this };
	wxy  = { this };
	wxz  = { this };
	wxw  = { this };
	wyx  = { this };
	wyy  = { this };
	wyz  = { this };
	wyw  = { this };
	wzx  = { this };
	wzy  = { this };
	wzz  = { this };
	wzw  = { this };
	wwx  = { this };
	wwy  = { this };
	wwz  = { this };
	www  = { this };

	xxxx = { this };
	xxxy = { this };
	xxxz = { this };
	xxxw = { this };
	xxyx = { this };
	xxyy = { this };
	xxyz = { this };
	xxyw = { this };
	xxzx = { this };
	xxzy = { this };
	xxzz = { this };
	xxzw = { this };
	xxwx = { this };
	xxwy = { this };
	xxwz = { this };
	xxww = { this };
	xyxx = { this };
	xyxy = { this };
	xyxz = { this };
	xyxw = { this };
	xyyx = { this };
	xyyy = { this };
	xyyz = { this };
	xyyw = { this };
	xyzx = { this };
	xyzy = { this };
	xyzz = { this };
	xyzw = { this };
	xywx = { this };
	xywy = { this };
	xywz = { this };
	xyww = { this };
	xzxx = { this };
	xzxy = { this };
	xzxz = { this };
	xzxw = { this };
	xzyx = { this };
	xzyy = { this };
	xzyz = { this };
	xzyw = { this };
	xzzx = { this };
	xzzy = { this };
	xzzz = { this };
	xzzw = { this };
	xzwx = { this };
	xzwy = { this };
	xzwz = { this };
	xzww = { this };
	xwxx = { this };
	xwxy = { this };
	xwxz = { this };
	xwxw = { this };
	xwyx = { this };
	xwyy = { this };
	xwyz = { this };
	xwyw = { this };
	xwzx = { this };
	xwzy = { this };
	xwzz = { this };
	xwzw = { this };
	xwwx = { this };
	xwwy = { this };
	xwwz = { this };
	xwww = { this };
	yxxx = { this };
	yxxy = { this };
	yxxz = { this };
	yxxw = { this };
	yxyx = { this };
	yxyy = { this };
	yxyz = { this };
	yxyw = { this };
	yxzx = { this };
	yxzy = { this };
	yxzz = { this };
	yxzw = { this };
	yxwx = { this };
	yxwy = { this };
	yxwz = { this };
	yxww = { this };
	yyxx = { this };
	yyxy = { this };
	yyxz = { this };
	yyxw = { this };
	yyyx = { this };
	yyyy = { this };
	yyyz = { this };
	yyyw = { this };
	yyzx = { this };
	yyzy = { this };
	yyzz = { this };
	yyzw = { this };
	yywx = { this };
	yywy = { this };
	yywz = { this };
	yyww = { this };
	yzxx = { this };
	yzxy = { this };
	yzxz = { this };
	yzxw = { this };
	yzyx = { this };
	yzyy = { this };
	yzyz = { this };
	yzyw = { this };
	yzzx = { this };
	yzzy = { this };
	yzzz = { this };
	yzzw = { this };
	yzwx = { this };
	yzwy = { this };
	yzwz = { this };
	yzww = { this };
	ywxx = { this };
	ywxy = { this };
	ywxz = { this };
	ywxw = { this };
	ywyx = { this };
	ywyy = { this };
	ywyz = { this };
	ywyw = { this };
	ywzx = { this };
	ywzy = { this };
	ywzz = { this };
	ywzw = { this };
	ywwx = { this };
	ywwy = { this };
	ywwz = { this };
	ywww = { this };
	zxxx = { this };
	zxxy = { this };
	zxxz = { this };
	zxxw = { this };
	zxyx = { this };
	zxyy = { this };
	zxyz = { this };
	zxyw = { this };
	zxzx = { this };
	zxzy = { this };
	zxzz = { this };
	zxzw = { this };
	zxwx = { this };
	zxwy = { this };
	zxwz = { this };
	zxww = { this };
	zyxx = { this };
	zyxy = { this };
	zyxz = { this };
	zyxw = { this };
	zyyx = { this };
	zyyy = { this };
	zyyz = { this };
	zyyw = { this };
	zyzx = { this };
	zyzy = { this };
	zyzz = { this };
	zyzw = { this };
	zywx = { this };
	zywy = { this };
	zywz = { this };
	zyww = { this };
	zzxx = { this };
	zzxy = { this };
	zzxz = { this };
	zzxw = { this };
	zzyx = { this };
	zzyy = { this };
	zzyz = { this };
	zzyw = { this };
	zzzx = { this };
	zzzy = { this };
	zzzz = { this };
	zzzw = { this };
	zzwx = { this };
	zzwy = { this };
	zzwz = { this };
	zzww = { this };
	zwxx = { this };
	zwxy = { this };
	zwxz = { this };
	zwxw = { this };
	zwyx = { this };
	zwyy = { this };
	zwyz = { this };
	zwyw = { this };
	zwzx = { this };
	zwzy = { this };
	zwzz = { this };
	zwzw = { this };
	zwwx = { this };
	zwwy = { this };
	zwwz = { this };
	zwww = { this };
	wxxx = { this };
	wxxy = { this };
	wxxz = { this };
	wxxw = { this };
	wxyx = { this };
	wxyy = { this };
	wxyz = { this };
	wxyw = { this };
	wxzx = { this };
	wxzy = { this };
	wxzz = { this };
	wxzw = { this };
	wxwx = { this };
	wxwy = { this };
	wxwz = { this };
	wxww = { this };
	wyxx = { this };
	wyxy = { this };
	wyxz = { this };
	wyxw = { this };
	wyyx = { this };
	wyyy = { this };
	wyyz = { this };
	wyyw = { this };
	wyzx = { this };
	wyzy = { this };
	wyzz = { this };
	wyzw = { this };
	wywx = { this };
	wywy = { this };
	wywz = { this };
	wyww = { this };
	wzxx = { this };
	wzxy = { this };
	wzxz = { this };
	wzxw = { this };
	wzyx = { this };
	wzyy = { this };
	wzyz = { this };
	wzyw = { this };
	wzzx = { this };
	wzzy = { this };
	wzzz = { this };
	wzzw = { this };
	wzwx = { this };
	wzwy = { this };
	wzwz = { this };
	wzww = { this };
	wwxx = { this };
	wwxy = { this };
	wwxz = { this };
	wwxw = { this };
	wwyx = { this };
	wwyy = { this };
	wwyz = { this };
	wwyw = { this };
	wwzx = { this };
	wwzy = { this };
	wwzz = { this };
	wwzw = { this };
	wwwx = { this };
	wwwy = { this };
	wwwz = { this };
	wwww = { this };

	r    = { this };
	g    = { this };
	b    = { this };
	a    = { this };

	rr   = { this };
	rg   = { this };
	rb   = { this };
	ra   = { this };
	gr   = { this };
	gg   = { this };
	gb   = { this };
	ga   = { this };
	br   = { this };
	bg   = { this };
	bb   = { this };
	ba   = { this };
	ar   = { this };
	ag   = { this };
	ab   = { this };
	aa   = { this };

	rrr  = { this };
	rrg  = { this };
	rrb  = { this };
	rra  = { this };
	rgr  = { this };
	rgg  = { this };
	rgb  = { this };
	rga  = { this };
	rbr  = { this };
	rbg  = { this };
	rbb  = { this };
	rba  = { this };
	rar  = { this };
	rag  = { this };
	rab  = { this };
	raa  = { this };
	grr  = { this };
	grg  = { this };
	grb  = { this };
	gra  = { this };
	ggr  = { this };
	ggg  = { this };
	ggb  = { this };
	gga  = { this };
	gbr  = { this };
	gbg  = { this };
	gbb  = { this };
	gba  = { this };
	gar  = { this };
	gag  = { this };
	gab  = { this };
	gaa  = { this };
	brr  = { this };
	brg  = { this };
	brb  = { this };
	bra  = { this };
	bgr  = { this };
	bgg  = { this };
	bgb  = { this };
	bga  = { this };
	bbr  = { this };
	bbg  = { this };
	bbb  = { this };
	bba  = { this };
	bar  = { this };
	bag  = { this };
	bab  = { this };
	baa  = { this };
	arr  = { this };
	arg  = { this };
	arb  = { this };
	ara  = { this };
	agr  = { this };
	agg  = { this };
	agb  = { this };
	aga  = { this };
	abr  = { this };
	abg  = { this };
	abb  = { this };
	aba  = { this };
	aar  = { this };
	aag  = { this };
	aab  = { this };
	aaa  = { this };

	rrrr = { this };
	rrrg = { this };
	rrrb = { this };
	rrra = { this };
	rrgr = { this };
	rrgg = { this };
	rrgb = { this };
	rrga = { this };
	rrbr = { this };
	rrbg = { this };
	rrbb = { this };
	rrba = { this };
	rrar = { this };
	rrag = { this };
	rrab = { this };
	rraa = { this };
	rgrr = { this };
	rgrg = { this };
	rgrb = { this };
	rgra = { this };
	rggr = { this };
	rggg = { this };
	rggb = { this };
	rgga = { this };
	rgbr = { this };
	rgbg = { this };
	rgbb = { this };
	rgba = { this };
	rgar = { this };
	rgag = { this };
	rgab = { this };
	rgaa = { this };
	rbrr = { this };
	rbrg = { this };
	rbrb = { this };
	rbra = { this };
	rbgr = { this };
	rbgg = { this };
	rbgb = { this };
	rbga = { this };
	rbbr = { this };
	rbbg = { this };
	rbbb = { this };
	rbba = { this };
	rbar = { this };
	rbag = { this };
	rbab = { this };
	rbaa = { this };
	rarr = { this };
	rarg = { this };
	rarb = { this };
	rara = { this };
	ragr = { this };
	ragg = { this };
	ragb = { this };
	raga = { this };
	rabr = { this };
	rabg = { this };
	rabb = { this };
	raba = { this };
	raar = { this };
	raag = { this };
	raab = { this };
	raaa = { this };
	grrr = { this };
	grrg = { this };
	grrb = { this };
	grra = { this };
	grgr = { this };
	grgg = { this };
	grgb = { this };
	grga = { this };
	grbr = { this };
	grbg = { this };
	grbb = { this };
	grba = { this };
	grar = { this };
	grag = { this };
	grab = { this };
	graa = { this };
	ggrr = { this };
	ggrg = { this };
	ggrb = { this };
	ggra = { this };
	gggr = { this };
	gggg = { this };
	gggb = { this };
	ggga = { this };
	ggbr = { this };
	ggbg = { this };
	ggbb = { this };
	ggba = { this };
	ggar = { this };
	ggag = { this };
	ggab = { this };
	ggaa = { this };
	gbrr = { this };
	gbrg = { this };
	gbrb = { this };
	gbra = { this };
	gbgr = { this };
	gbgg = { this };
	gbgb = { this };
	gbga = { this };
	gbbr = { this };
	gbbg = { this };
	gbbb = { this };
	gbba = { this };
	gbar = { this };
	gbag = { this };
	gbab = { this };
	gbaa = { this };
	garr = { this };
	garg = { this };
	garb = { this };
	gara = { this };
	gagr = { this };
	gagg = { this };
	gagb = { this };
	gaga = { this };
	gabr = { this };
	gabg = { this };
	gabb = { this };
	gaba = { this };
	gaar = { this };
	gaag = { this };
	gaab = { this };
	gaaa = { this };
	brrr = { this };
	brrg = { this };
	brrb = { this };
	brra = { this };
	brgr = { this };
	brgg = { this };
	brgb = { this };
	brga = { this };
	brbr = { this };
	brbg = { this };
	brbb = { this };
	brba = { this };
	brar = { this };
	brag = { this };
	brab = { this };
	braa = { this };
	bgrr = { this };
	bgrg = { this };
	bgrb = { this };
	bgra = { this };
	bggr = { this };
	bggg = { this };
	bggb = { this };
	bgga = { this };
	bgbr = { this };
	bgbg = { this };
	bgbb = { this };
	bgba = { this };
	bgar = { this };
	bgag = { this };
	bgab = { this };
	bgaa = { this };
	bbrr = { this };
	bbrg = { this };
	bbrb = { this };
	bbra = { this };
	bbgr = { this };
	bbgg = { this };
	bbgb = { this };
	bbga = { this };
	bbbr = { this };
	bbbg = { this };
	bbbb = { this };
	bbba = { this };
	bbar = { this };
	bbag = { this };
	bbab = { this };
	bbaa = { this };
	barr = { this };
	barg = { this };
	barb = { this };
	bara = { this };
	bagr = { this };
	bagg = { this };
	bagb = { this };
	baga = { this };
	babr = { this };
	babg = { this };
	babb = { this };
	baba = { this };
	baar = { this };
	baag = { this };
	baab = { this };
	baaa = { this };
	arrr = { this };
	arrg = { this };
	arrb = { this };
	arra = { this };
	argr = { this };
	argg = { this };
	argb = { this };
	arga = { this };
	arbr = { this };
	arbg = { this };
	arbb = { this };
	arba = { this };
	arar = { this };
	arag = { this };
	arab = { this };
	araa = { this };
	agrr = { this };
	agrg = { this };
	agrb = { this };
	agra = { this };
	aggr = { this };
	aggg = { this };
	aggb = { this };
	agga = { this };
	agbr = { this };
	agbg = { this };
	agbb = { this };
	agba = { this };
	agar = { this };
	agag = { this };
	agab = { this };
	agaa = { this };
	abrr = { this };
	abrg = { this };
	abrb = { this };
	abra = { this };
	abgr = { this };
	abgg = { this };
	abgb = { this };
	abga = { this };
	abbr = { this };
	abbg = { this };
	abbb = { this };
	abba = { this };
	abar = { this };
	abag = { this };
	abab = { this };
	abaa = { this };
	aarr = { this };
	aarg = { this };
	aarb = { this };
	aara = { this };
	aagr = { this };
	aagg = { this };
	aagb = { this };
	aaga = { this };
	aabr = { this };
	aabg = { this };
	aabb = { this };
	aaba = { this };
	aaar = { this };
	aaag = { this };
	aaab = { this };
	aaaa = { this };
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
