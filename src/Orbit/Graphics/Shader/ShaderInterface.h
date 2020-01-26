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

#pragma once
#include "Orbit/Graphics/Shader/VertexLayout.h"

#include <string_view>
#include <string>

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS ShaderInterface
{
public:

	enum class VariableType
	{
		Unknown = 0,

		Float,
		Vec2,
		Vec3,
		Vec4,
		Mat4,
	};

public:

	ShaderInterface( void );
	virtual ~ShaderInterface( void );

public:

	std::string  GetSource      ( void );
	VertexLayout GetVertexLayout( void ) const;

protected:

	class Variable;

	class ORB_API_GRAPHICS VariableDummy
	{
		friend class Variable;

	public:

		void operator*=( const Variable& rhs ) const;

	public:

		operator Variable( void ) const;

	public:

		Variable*   parent;
		const char* value;

	};

	class ORB_API_GRAPHICS Variable
	{
		friend class ShaderInterface;

	public:

		Variable( void );
		explicit Variable( const Variable& );
		Variable( Variable&& );
		Variable( double );

	protected:

		explicit Variable( std::string_view );

	public:

		Variable operator*( const Variable& ) const;
		Variable operator+( const Variable& ) const;
		Variable operator-( void )     const;

		void operator= ( const Variable& );
		void operator+=( const Variable& );
		void operator*=( const Variable& );

	private:

		void StoreValue ( void );
		void InitDummies( void );

	private:

		std::string  m_value;
		VariableType m_type   = VariableType::Unknown;
		bool         m_stored = false;
		mutable bool m_used   = false;

	public:

		VariableDummy x;
		VariableDummy y;
		VariableDummy z;
		VariableDummy w;

		VariableDummy xx;
		VariableDummy xy;
		VariableDummy xz;
		VariableDummy xw;
		VariableDummy yx;
		VariableDummy yy;
		VariableDummy yz;
		VariableDummy yw;
		VariableDummy zx;
		VariableDummy zy;
		VariableDummy zz;
		VariableDummy zw;
		VariableDummy wx;
		VariableDummy wy;
		VariableDummy wz;
		VariableDummy ww;

		VariableDummy xxx;
		VariableDummy xxy;
		VariableDummy xxz;
		VariableDummy xxw;
		VariableDummy xyx;
		VariableDummy xyy;
		VariableDummy xyz;
		VariableDummy xyw;
		VariableDummy xzx;
		VariableDummy xzy;
		VariableDummy xzz;
		VariableDummy xzw;
		VariableDummy xwx;
		VariableDummy xwy;
		VariableDummy xwz;
		VariableDummy xww;
		VariableDummy yxx;
		VariableDummy yxy;
		VariableDummy yxz;
		VariableDummy yxw;
		VariableDummy yyx;
		VariableDummy yyy;
		VariableDummy yyz;
		VariableDummy yyw;
		VariableDummy yzx;
		VariableDummy yzy;
		VariableDummy yzz;
		VariableDummy yzw;
		VariableDummy ywx;
		VariableDummy ywy;
		VariableDummy ywz;
		VariableDummy yww;
		VariableDummy zxx;
		VariableDummy zxy;
		VariableDummy zxz;
		VariableDummy zxw;
		VariableDummy zyx;
		VariableDummy zyy;
		VariableDummy zyz;
		VariableDummy zyw;
		VariableDummy zzx;
		VariableDummy zzy;
		VariableDummy zzz;
		VariableDummy zzw;
		VariableDummy zwx;
		VariableDummy zwy;
		VariableDummy zwz;
		VariableDummy zww;
		VariableDummy wxx;
		VariableDummy wxy;
		VariableDummy wxz;
		VariableDummy wxw;
		VariableDummy wyx;
		VariableDummy wyy;
		VariableDummy wyz;
		VariableDummy wyw;
		VariableDummy wzx;
		VariableDummy wzy;
		VariableDummy wzz;
		VariableDummy wzw;
		VariableDummy wwx;
		VariableDummy wwy;
		VariableDummy wwz;
		VariableDummy www;

		VariableDummy xxxx;
		VariableDummy xxxy;
		VariableDummy xxxz;
		VariableDummy xxxw;
		VariableDummy xxyx;
		VariableDummy xxyy;
		VariableDummy xxyz;
		VariableDummy xxyw;
		VariableDummy xxzx;
		VariableDummy xxzy;
		VariableDummy xxzz;
		VariableDummy xxzw;
		VariableDummy xxwx;
		VariableDummy xxwy;
		VariableDummy xxwz;
		VariableDummy xxww;
		VariableDummy xyxx;
		VariableDummy xyxy;
		VariableDummy xyxz;
		VariableDummy xyxw;
		VariableDummy xyyx;
		VariableDummy xyyy;
		VariableDummy xyyz;
		VariableDummy xyyw;
		VariableDummy xyzx;
		VariableDummy xyzy;
		VariableDummy xyzz;
		VariableDummy xyzw;
		VariableDummy xywx;
		VariableDummy xywy;
		VariableDummy xywz;
		VariableDummy xyww;
		VariableDummy xzxx;
		VariableDummy xzxy;
		VariableDummy xzxz;
		VariableDummy xzxw;
		VariableDummy xzyx;
		VariableDummy xzyy;
		VariableDummy xzyz;
		VariableDummy xzyw;
		VariableDummy xzzx;
		VariableDummy xzzy;
		VariableDummy xzzz;
		VariableDummy xzzw;
		VariableDummy xzwx;
		VariableDummy xzwy;
		VariableDummy xzwz;
		VariableDummy xzww;
		VariableDummy xwxx;
		VariableDummy xwxy;
		VariableDummy xwxz;
		VariableDummy xwxw;
		VariableDummy xwyx;
		VariableDummy xwyy;
		VariableDummy xwyz;
		VariableDummy xwyw;
		VariableDummy xwzx;
		VariableDummy xwzy;
		VariableDummy xwzz;
		VariableDummy xwzw;
		VariableDummy xwwx;
		VariableDummy xwwy;
		VariableDummy xwwz;
		VariableDummy xwww;
		VariableDummy yxxx;
		VariableDummy yxxy;
		VariableDummy yxxz;
		VariableDummy yxxw;
		VariableDummy yxyx;
		VariableDummy yxyy;
		VariableDummy yxyz;
		VariableDummy yxyw;
		VariableDummy yxzx;
		VariableDummy yxzy;
		VariableDummy yxzz;
		VariableDummy yxzw;
		VariableDummy yxwx;
		VariableDummy yxwy;
		VariableDummy yxwz;
		VariableDummy yxww;
		VariableDummy yyxx;
		VariableDummy yyxy;
		VariableDummy yyxz;
		VariableDummy yyxw;
		VariableDummy yyyx;
		VariableDummy yyyy;
		VariableDummy yyyz;
		VariableDummy yyyw;
		VariableDummy yyzx;
		VariableDummy yyzy;
		VariableDummy yyzz;
		VariableDummy yyzw;
		VariableDummy yywx;
		VariableDummy yywy;
		VariableDummy yywz;
		VariableDummy yyww;
		VariableDummy yzxx;
		VariableDummy yzxy;
		VariableDummy yzxz;
		VariableDummy yzxw;
		VariableDummy yzyx;
		VariableDummy yzyy;
		VariableDummy yzyz;
		VariableDummy yzyw;
		VariableDummy yzzx;
		VariableDummy yzzy;
		VariableDummy yzzz;
		VariableDummy yzzw;
		VariableDummy yzwx;
		VariableDummy yzwy;
		VariableDummy yzwz;
		VariableDummy yzww;
		VariableDummy ywxx;
		VariableDummy ywxy;
		VariableDummy ywxz;
		VariableDummy ywxw;
		VariableDummy ywyx;
		VariableDummy ywyy;
		VariableDummy ywyz;
		VariableDummy ywyw;
		VariableDummy ywzx;
		VariableDummy ywzy;
		VariableDummy ywzz;
		VariableDummy ywzw;
		VariableDummy ywwx;
		VariableDummy ywwy;
		VariableDummy ywwz;
		VariableDummy ywww;
		VariableDummy zxxx;
		VariableDummy zxxy;
		VariableDummy zxxz;
		VariableDummy zxxw;
		VariableDummy zxyx;
		VariableDummy zxyy;
		VariableDummy zxyz;
		VariableDummy zxyw;
		VariableDummy zxzx;
		VariableDummy zxzy;
		VariableDummy zxzz;
		VariableDummy zxzw;
		VariableDummy zxwx;
		VariableDummy zxwy;
		VariableDummy zxwz;
		VariableDummy zxww;
		VariableDummy zyxx;
		VariableDummy zyxy;
		VariableDummy zyxz;
		VariableDummy zyxw;
		VariableDummy zyyx;
		VariableDummy zyyy;
		VariableDummy zyyz;
		VariableDummy zyyw;
		VariableDummy zyzx;
		VariableDummy zyzy;
		VariableDummy zyzz;
		VariableDummy zyzw;
		VariableDummy zywx;
		VariableDummy zywy;
		VariableDummy zywz;
		VariableDummy zyww;
		VariableDummy zzxx;
		VariableDummy zzxy;
		VariableDummy zzxz;
		VariableDummy zzxw;
		VariableDummy zzyx;
		VariableDummy zzyy;
		VariableDummy zzyz;
		VariableDummy zzyw;
		VariableDummy zzzx;
		VariableDummy zzzy;
		VariableDummy zzzz;
		VariableDummy zzzw;
		VariableDummy zzwx;
		VariableDummy zzwy;
		VariableDummy zzwz;
		VariableDummy zzww;
		VariableDummy zwxx;
		VariableDummy zwxy;
		VariableDummy zwxz;
		VariableDummy zwxw;
		VariableDummy zwyx;
		VariableDummy zwyy;
		VariableDummy zwyz;
		VariableDummy zwyw;
		VariableDummy zwzx;
		VariableDummy zwzy;
		VariableDummy zwzz;
		VariableDummy zwzw;
		VariableDummy zwwx;
		VariableDummy zwwy;
		VariableDummy zwwz;
		VariableDummy zwww;
		VariableDummy wxxx;
		VariableDummy wxxy;
		VariableDummy wxxz;
		VariableDummy wxxw;
		VariableDummy wxyx;
		VariableDummy wxyy;
		VariableDummy wxyz;
		VariableDummy wxyw;
		VariableDummy wxzx;
		VariableDummy wxzy;
		VariableDummy wxzz;
		VariableDummy wxzw;
		VariableDummy wxwx;
		VariableDummy wxwy;
		VariableDummy wxwz;
		VariableDummy wxww;
		VariableDummy wyxx;
		VariableDummy wyxy;
		VariableDummy wyxz;
		VariableDummy wyxw;
		VariableDummy wyyx;
		VariableDummy wyyy;
		VariableDummy wyyz;
		VariableDummy wyyw;
		VariableDummy wyzx;
		VariableDummy wyzy;
		VariableDummy wyzz;
		VariableDummy wyzw;
		VariableDummy wywx;
		VariableDummy wywy;
		VariableDummy wywz;
		VariableDummy wyww;
		VariableDummy wzxx;
		VariableDummy wzxy;
		VariableDummy wzxz;
		VariableDummy wzxw;
		VariableDummy wzyx;
		VariableDummy wzyy;
		VariableDummy wzyz;
		VariableDummy wzyw;
		VariableDummy wzzx;
		VariableDummy wzzy;
		VariableDummy wzzz;
		VariableDummy wzzw;
		VariableDummy wzwx;
		VariableDummy wzwy;
		VariableDummy wzwz;
		VariableDummy wzww;
		VariableDummy wwxx;
		VariableDummy wwxy;
		VariableDummy wwxz;
		VariableDummy wwxw;
		VariableDummy wwyx;
		VariableDummy wwyy;
		VariableDummy wwyz;
		VariableDummy wwyw;
		VariableDummy wwzx;
		VariableDummy wwzy;
		VariableDummy wwzz;
		VariableDummy wwzw;
		VariableDummy wwwx;
		VariableDummy wwwy;
		VariableDummy wwwz;
		VariableDummy wwww;

		VariableDummy r;
		VariableDummy g;
		VariableDummy b;
		VariableDummy a;

		VariableDummy rr;
		VariableDummy rg;
		VariableDummy rb;
		VariableDummy ra;
		VariableDummy gr;
		VariableDummy gg;
		VariableDummy gb;
		VariableDummy ga;
		VariableDummy br;
		VariableDummy bg;
		VariableDummy bb;
		VariableDummy ba;
		VariableDummy ar;
		VariableDummy ag;
		VariableDummy ab;
		VariableDummy aa;

		VariableDummy rrr;
		VariableDummy rrg;
		VariableDummy rrb;
		VariableDummy rra;
		VariableDummy rgr;
		VariableDummy rgg;
		VariableDummy rgb;
		VariableDummy rga;
		VariableDummy rbr;
		VariableDummy rbg;
		VariableDummy rbb;
		VariableDummy rba;
		VariableDummy rar;
		VariableDummy rag;
		VariableDummy rab;
		VariableDummy raa;
		VariableDummy grr;
		VariableDummy grg;
		VariableDummy grb;
		VariableDummy gra;
		VariableDummy ggr;
		VariableDummy ggg;
		VariableDummy ggb;
		VariableDummy gga;
		VariableDummy gbr;
		VariableDummy gbg;
		VariableDummy gbb;
		VariableDummy gba;
		VariableDummy gar;
		VariableDummy gag;
		VariableDummy gab;
		VariableDummy gaa;
		VariableDummy brr;
		VariableDummy brg;
		VariableDummy brb;
		VariableDummy bra;
		VariableDummy bgr;
		VariableDummy bgg;
		VariableDummy bgb;
		VariableDummy bga;
		VariableDummy bbr;
		VariableDummy bbg;
		VariableDummy bbb;
		VariableDummy bba;
		VariableDummy bar;
		VariableDummy bag;
		VariableDummy bab;
		VariableDummy baa;
		VariableDummy arr;
		VariableDummy arg;
		VariableDummy arb;
		VariableDummy ara;
		VariableDummy agr;
		VariableDummy agg;
		VariableDummy agb;
		VariableDummy aga;
		VariableDummy abr;
		VariableDummy abg;
		VariableDummy abb;
		VariableDummy aba;
		VariableDummy aar;
		VariableDummy aag;
		VariableDummy aab;
		VariableDummy aaa;

		VariableDummy rrrr;
		VariableDummy rrrg;
		VariableDummy rrrb;
		VariableDummy rrra;
		VariableDummy rrgr;
		VariableDummy rrgg;
		VariableDummy rrgb;
		VariableDummy rrga;
		VariableDummy rrbr;
		VariableDummy rrbg;
		VariableDummy rrbb;
		VariableDummy rrba;
		VariableDummy rrar;
		VariableDummy rrag;
		VariableDummy rrab;
		VariableDummy rraa;
		VariableDummy rgrr;
		VariableDummy rgrg;
		VariableDummy rgrb;
		VariableDummy rgra;
		VariableDummy rggr;
		VariableDummy rggg;
		VariableDummy rggb;
		VariableDummy rgga;
		VariableDummy rgbr;
		VariableDummy rgbg;
		VariableDummy rgbb;
		VariableDummy rgba;
		VariableDummy rgar;
		VariableDummy rgag;
		VariableDummy rgab;
		VariableDummy rgaa;
		VariableDummy rbrr;
		VariableDummy rbrg;
		VariableDummy rbrb;
		VariableDummy rbra;
		VariableDummy rbgr;
		VariableDummy rbgg;
		VariableDummy rbgb;
		VariableDummy rbga;
		VariableDummy rbbr;
		VariableDummy rbbg;
		VariableDummy rbbb;
		VariableDummy rbba;
		VariableDummy rbar;
		VariableDummy rbag;
		VariableDummy rbab;
		VariableDummy rbaa;
		VariableDummy rarr;
		VariableDummy rarg;
		VariableDummy rarb;
		VariableDummy rara;
		VariableDummy ragr;
		VariableDummy ragg;
		VariableDummy ragb;
		VariableDummy raga;
		VariableDummy rabr;
		VariableDummy rabg;
		VariableDummy rabb;
		VariableDummy raba;
		VariableDummy raar;
		VariableDummy raag;
		VariableDummy raab;
		VariableDummy raaa;
		VariableDummy grrr;
		VariableDummy grrg;
		VariableDummy grrb;
		VariableDummy grra;
		VariableDummy grgr;
		VariableDummy grgg;
		VariableDummy grgb;
		VariableDummy grga;
		VariableDummy grbr;
		VariableDummy grbg;
		VariableDummy grbb;
		VariableDummy grba;
		VariableDummy grar;
		VariableDummy grag;
		VariableDummy grab;
		VariableDummy graa;
		VariableDummy ggrr;
		VariableDummy ggrg;
		VariableDummy ggrb;
		VariableDummy ggra;
		VariableDummy gggr;
		VariableDummy gggg;
		VariableDummy gggb;
		VariableDummy ggga;
		VariableDummy ggbr;
		VariableDummy ggbg;
		VariableDummy ggbb;
		VariableDummy ggba;
		VariableDummy ggar;
		VariableDummy ggag;
		VariableDummy ggab;
		VariableDummy ggaa;
		VariableDummy gbrr;
		VariableDummy gbrg;
		VariableDummy gbrb;
		VariableDummy gbra;
		VariableDummy gbgr;
		VariableDummy gbgg;
		VariableDummy gbgb;
		VariableDummy gbga;
		VariableDummy gbbr;
		VariableDummy gbbg;
		VariableDummy gbbb;
		VariableDummy gbba;
		VariableDummy gbar;
		VariableDummy gbag;
		VariableDummy gbab;
		VariableDummy gbaa;
		VariableDummy garr;
		VariableDummy garg;
		VariableDummy garb;
		VariableDummy gara;
		VariableDummy gagr;
		VariableDummy gagg;
		VariableDummy gagb;
		VariableDummy gaga;
		VariableDummy gabr;
		VariableDummy gabg;
		VariableDummy gabb;
		VariableDummy gaba;
		VariableDummy gaar;
		VariableDummy gaag;
		VariableDummy gaab;
		VariableDummy gaaa;
		VariableDummy brrr;
		VariableDummy brrg;
		VariableDummy brrb;
		VariableDummy brra;
		VariableDummy brgr;
		VariableDummy brgg;
		VariableDummy brgb;
		VariableDummy brga;
		VariableDummy brbr;
		VariableDummy brbg;
		VariableDummy brbb;
		VariableDummy brba;
		VariableDummy brar;
		VariableDummy brag;
		VariableDummy brab;
		VariableDummy braa;
		VariableDummy bgrr;
		VariableDummy bgrg;
		VariableDummy bgrb;
		VariableDummy bgra;
		VariableDummy bggr;
		VariableDummy bggg;
		VariableDummy bggb;
		VariableDummy bgga;
		VariableDummy bgbr;
		VariableDummy bgbg;
		VariableDummy bgbb;
		VariableDummy bgba;
		VariableDummy bgar;
		VariableDummy bgag;
		VariableDummy bgab;
		VariableDummy bgaa;
		VariableDummy bbrr;
		VariableDummy bbrg;
		VariableDummy bbrb;
		VariableDummy bbra;
		VariableDummy bbgr;
		VariableDummy bbgg;
		VariableDummy bbgb;
		VariableDummy bbga;
		VariableDummy bbbr;
		VariableDummy bbbg;
		VariableDummy bbbb;
		VariableDummy bbba;
		VariableDummy bbar;
		VariableDummy bbag;
		VariableDummy bbab;
		VariableDummy bbaa;
		VariableDummy barr;
		VariableDummy barg;
		VariableDummy barb;
		VariableDummy bara;
		VariableDummy bagr;
		VariableDummy bagg;
		VariableDummy bagb;
		VariableDummy baga;
		VariableDummy babr;
		VariableDummy babg;
		VariableDummy babb;
		VariableDummy baba;
		VariableDummy baar;
		VariableDummy baag;
		VariableDummy baab;
		VariableDummy baaa;
		VariableDummy arrr;
		VariableDummy arrg;
		VariableDummy arrb;
		VariableDummy arra;
		VariableDummy argr;
		VariableDummy argg;
		VariableDummy argb;
		VariableDummy arga;
		VariableDummy arbr;
		VariableDummy arbg;
		VariableDummy arbb;
		VariableDummy arba;
		VariableDummy arar;
		VariableDummy arag;
		VariableDummy arab;
		VariableDummy araa;
		VariableDummy agrr;
		VariableDummy agrg;
		VariableDummy agrb;
		VariableDummy agra;
		VariableDummy aggr;
		VariableDummy aggg;
		VariableDummy aggb;
		VariableDummy agga;
		VariableDummy agbr;
		VariableDummy agbg;
		VariableDummy agbb;
		VariableDummy agba;
		VariableDummy agar;
		VariableDummy agag;
		VariableDummy agab;
		VariableDummy agaa;
		VariableDummy abrr;
		VariableDummy abrg;
		VariableDummy abrb;
		VariableDummy abra;
		VariableDummy abgr;
		VariableDummy abgg;
		VariableDummy abgb;
		VariableDummy abga;
		VariableDummy abbr;
		VariableDummy abbg;
		VariableDummy abbb;
		VariableDummy abba;
		VariableDummy abar;
		VariableDummy abag;
		VariableDummy abab;
		VariableDummy abaa;
		VariableDummy aarr;
		VariableDummy aarg;
		VariableDummy aarb;
		VariableDummy aara;
		VariableDummy aagr;
		VariableDummy aagg;
		VariableDummy aagb;
		VariableDummy aaga;
		VariableDummy aabr;
		VariableDummy aabg;
		VariableDummy aabb;
		VariableDummy aaba;
		VariableDummy aaar;
		VariableDummy aaag;
		VariableDummy aaab;
		VariableDummy aaaa;

	};

	class ORB_API_GRAPHICS Float : public Variable
	{
	public:

		Float( const Variable& );

	};

	class ORB_API_GRAPHICS Vec2 : public Variable
	{
	public:

		Vec2( const Variable& );
		Vec2( const Variable&, const Variable& );

	};

	class ORB_API_GRAPHICS Vec3 : public Variable
	{
	public:

		Vec3( const Variable& );
		Vec3( const Variable&, const Variable& );
		Vec3( const Variable&, const Variable&, const Variable& );

	};

	class ORB_API_GRAPHICS Vec4 : public Variable
	{
	public:

		Vec4( const Variable& );
		Vec4( const Variable&, const Variable& );
		Vec4( const Variable&, const Variable&, const Variable& );
		Vec4( const Variable&, const Variable&, const Variable&, const Variable& );

	};

	class ORB_API_GRAPHICS Mat4 : public Variable
	{
	public:

		Mat4( const Variable& );

	};

	class ORB_API_GRAPHICS Sampler : public Variable
	{
	public:

		Sampler( void );

	};

	template< VertexComponent VC >
	class VaryingHelper;

	class ORB_API_GRAPHICS Varying : public Variable
	{
	public:

		using Position = VaryingHelper< VertexComponent::Position >;
		using Normal   = VaryingHelper< VertexComponent::Normal >;
		using Color    = VaryingHelper< VertexComponent::Color >;
		using TexCoord = VaryingHelper< VertexComponent::TexCoord >;

		using Variable::operator=;

	public:

		Varying( VertexComponent );

	};

	template< VertexComponent VC >
	class VaryingHelper : public Varying
	{
	public:

		using Varying::operator=;

	public:

		VaryingHelper( void ) : Varying( VC ) { }

	};

	template< VertexComponent VC >
	class AttributeHelper;

	class ORB_API_GRAPHICS Attribute : public Variable
	{
	public:

		using Position = AttributeHelper< VertexComponent::Position >;
		using Normal   = AttributeHelper< VertexComponent::Normal >;
		using Color    = AttributeHelper< VertexComponent::Color >;
		using TexCoord = AttributeHelper< VertexComponent::TexCoord >;

		using Variable::operator=;

	public:

		Attribute( VertexComponent );

	};

	template< VertexComponent VC >
	class AttributeHelper : public Attribute
	{
	public:

		AttributeHelper( void ) : Attribute( VC ) { }

	};

	class ORB_API_GRAPHICS UniformBase : public Variable
	{
	public:

		UniformBase( VariableType );

	};

	template< typename T >
	class Uniform : public UniformBase
	{
	public:

		Uniform( void );

	};

protected:

	virtual Vec4 VSMain( void ) = 0;
	virtual Vec4 PSMain( void ) = 0;

protected:

	Variable Transpose( const Variable& );
	Variable Sample   ( const Variable&, const Variable& );
	Variable Dot      ( const Variable&, const Variable& );

private:

	void GenerateSourceHLSL( void );
	void GenerateSourceGLSL( void );

private:

	std::string                 m_source_code;
	std::vector< UniformBase* > m_uniforms;
	VertexLayout                m_attribute_layout;
	VertexLayout                m_varying_layout;
	uint32_t                    m_sampler_count;

};

template<>
inline ShaderInterface::Uniform< ShaderInterface::Float >::Uniform( void ) : UniformBase( VariableType::Float ) { }

template<>
inline ShaderInterface::Uniform< ShaderInterface::Vec2 >::Uniform( void ) : UniformBase( VariableType::Vec2 ) { }

template<>
inline ShaderInterface::Uniform< ShaderInterface::Vec3 >::Uniform( void ) : UniformBase( VariableType::Vec3 ) { }

template<>
inline ShaderInterface::Uniform< ShaderInterface::Vec4 >::Uniform( void ) : UniformBase( VariableType::Vec4 ) { }

template<>
inline ShaderInterface::Uniform< ShaderInterface::Mat4 >::Uniform( void ) : UniformBase( VariableType::Mat4 ) { }

ORB_NAMESPACE_END
