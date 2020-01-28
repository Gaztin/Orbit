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
#include "Orbit/Core/Utility/StringLiteral.h"
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

	template< char... ProxyName >
	class Proxy
	{
		friend class Variable;

	public:

		static constexpr StringLiteral< ProxyName... > proxy_name{ };

	public:

		static constexpr VariableType GetVariableType( void )
		{
			switch( proxy_name.length )
			{
				case 1: { return VariableType::Float; }
				case 2: { return VariableType::Vec2;  }
				case 3: { return VariableType::Vec3;  }
				case 4: { return VariableType::Vec4;  }
			}

			return VariableType::Unknown;
		}

	public:

		void operator*=( const Variable& rhs ) const
		{
			parent->StoreValue();

			static_cast< Variable >( *this ) *= rhs;
		}

		operator Variable( void ) const
		{
			Variable proxy_variable( parent->GetValue() + "." + proxy_name.value, GetVariableType() );

			/* If parent is stored, then the proxy can be considered stored too.
			 * Otherwise, we'd not be able to manipulate the proxies within variables.
			 * `foo.rgb *= 0.5;` would become `vec3 local = foo.rgb; local *= 0.5;` and `foo` would
			 * be left unchanged. */
			proxy_variable.m_stored = parent->m_stored;

			return proxy_variable;
		}

	public:

		Variable* parent;

	};

#define ORB_SHADER_PROXY( X )                       \
    decltype( ORB_NAMESPACE ShaderInterface::Proxy< \
        ORB_NAMESPACE StringLiteralGrab< 0 >( X ),  \
        ORB_NAMESPACE StringLiteralGrab< 1 >( X ),  \
        ORB_NAMESPACE StringLiteralGrab< 2 >( X ),  \
        ORB_NAMESPACE StringLiteralGrab< 3 >( X ) >() )

	class ORB_API_GRAPHICS Variable
	{
		friend class ShaderInterface;

	public:

		Variable( void );
		explicit Variable( const Variable& );
		Variable( Variable&& );
		Variable( double );

	protected:

		explicit Variable( std::string_view, VariableType type = VariableType::Unknown );

	public:

		virtual std::string GetValue( void ) const { return m_value; }

	public:

		Variable operator*( const Variable& ) const;
		Variable operator+( const Variable& ) const;
		Variable operator-( void )     const;

		void operator= ( const Variable& );
		void operator+=( const Variable& );
		void operator*=( const Variable& );

	private:

		/* Stores the value in a local variable. Useful when we want to manipulate proxies within a
		 * variable, since `Vec2(1.0, 0.5).g *= 2.0;` is ill-behaved. */
		void StoreValue( void );

		/* Initializes the proxies within this variable. */
		void InitProxies( void );

	private:

		std::string  m_value;
		VariableType m_type   = VariableType::Unknown;
		bool         m_stored = false;
		mutable bool m_used   = false;

	public:

		ORB_SHADER_PROXY( "x" ) x;
		ORB_SHADER_PROXY( "y" ) y;
		ORB_SHADER_PROXY( "z" ) z;
		ORB_SHADER_PROXY( "w" ) w;

		ORB_SHADER_PROXY( "xx" ) xx;
		ORB_SHADER_PROXY( "xy" ) xy;
		ORB_SHADER_PROXY( "xz" ) xz;
		ORB_SHADER_PROXY( "xw" ) xw;
		ORB_SHADER_PROXY( "yx" ) yx;
		ORB_SHADER_PROXY( "yy" ) yy;
		ORB_SHADER_PROXY( "yz" ) yz;
		ORB_SHADER_PROXY( "yw" ) yw;
		ORB_SHADER_PROXY( "zx" ) zx;
		ORB_SHADER_PROXY( "zy" ) zy;
		ORB_SHADER_PROXY( "zz" ) zz;
		ORB_SHADER_PROXY( "zw" ) zw;
		ORB_SHADER_PROXY( "wx" ) wx;
		ORB_SHADER_PROXY( "wy" ) wy;
		ORB_SHADER_PROXY( "wz" ) wz;
		ORB_SHADER_PROXY( "ww" ) ww;

		ORB_SHADER_PROXY( "xxx" ) xxx;
		ORB_SHADER_PROXY( "xxy" ) xxy;
		ORB_SHADER_PROXY( "xxz" ) xxz;
		ORB_SHADER_PROXY( "xxw" ) xxw;
		ORB_SHADER_PROXY( "xyx" ) xyx;
		ORB_SHADER_PROXY( "xyy" ) xyy;
		ORB_SHADER_PROXY( "xyz" ) xyz;
		ORB_SHADER_PROXY( "xyw" ) xyw;
		ORB_SHADER_PROXY( "xzx" ) xzx;
		ORB_SHADER_PROXY( "xzy" ) xzy;
		ORB_SHADER_PROXY( "xzz" ) xzz;
		ORB_SHADER_PROXY( "xzw" ) xzw;
		ORB_SHADER_PROXY( "xwx" ) xwx;
		ORB_SHADER_PROXY( "xwy" ) xwy;
		ORB_SHADER_PROXY( "xwz" ) xwz;
		ORB_SHADER_PROXY( "xww" ) xww;
		ORB_SHADER_PROXY( "yxx" ) yxx;
		ORB_SHADER_PROXY( "yxy" ) yxy;
		ORB_SHADER_PROXY( "yxz" ) yxz;
		ORB_SHADER_PROXY( "yxw" ) yxw;
		ORB_SHADER_PROXY( "yyx" ) yyx;
		ORB_SHADER_PROXY( "yyy" ) yyy;
		ORB_SHADER_PROXY( "yyz" ) yyz;
		ORB_SHADER_PROXY( "yyw" ) yyw;
		ORB_SHADER_PROXY( "yzx" ) yzx;
		ORB_SHADER_PROXY( "yzy" ) yzy;
		ORB_SHADER_PROXY( "yzz" ) yzz;
		ORB_SHADER_PROXY( "yzw" ) yzw;
		ORB_SHADER_PROXY( "ywx" ) ywx;
		ORB_SHADER_PROXY( "ywy" ) ywy;
		ORB_SHADER_PROXY( "ywz" ) ywz;
		ORB_SHADER_PROXY( "yww" ) yww;
		ORB_SHADER_PROXY( "zxx" ) zxx;
		ORB_SHADER_PROXY( "zxy" ) zxy;
		ORB_SHADER_PROXY( "zxz" ) zxz;
		ORB_SHADER_PROXY( "zxw" ) zxw;
		ORB_SHADER_PROXY( "zyx" ) zyx;
		ORB_SHADER_PROXY( "zyy" ) zyy;
		ORB_SHADER_PROXY( "zyz" ) zyz;
		ORB_SHADER_PROXY( "zyw" ) zyw;
		ORB_SHADER_PROXY( "zzx" ) zzx;
		ORB_SHADER_PROXY( "zzy" ) zzy;
		ORB_SHADER_PROXY( "zzz" ) zzz;
		ORB_SHADER_PROXY( "zzw" ) zzw;
		ORB_SHADER_PROXY( "zwx" ) zwx;
		ORB_SHADER_PROXY( "zwy" ) zwy;
		ORB_SHADER_PROXY( "zwz" ) zwz;
		ORB_SHADER_PROXY( "zww" ) zww;
		ORB_SHADER_PROXY( "wxx" ) wxx;
		ORB_SHADER_PROXY( "wxy" ) wxy;
		ORB_SHADER_PROXY( "wxz" ) wxz;
		ORB_SHADER_PROXY( "wxw" ) wxw;
		ORB_SHADER_PROXY( "wyx" ) wyx;
		ORB_SHADER_PROXY( "wyy" ) wyy;
		ORB_SHADER_PROXY( "wyz" ) wyz;
		ORB_SHADER_PROXY( "wyw" ) wyw;
		ORB_SHADER_PROXY( "wzx" ) wzx;
		ORB_SHADER_PROXY( "wzy" ) wzy;
		ORB_SHADER_PROXY( "wzz" ) wzz;
		ORB_SHADER_PROXY( "wzw" ) wzw;
		ORB_SHADER_PROXY( "wwx" ) wwx;
		ORB_SHADER_PROXY( "wwy" ) wwy;
		ORB_SHADER_PROXY( "wwz" ) wwz;
		ORB_SHADER_PROXY( "www" ) www;

		ORB_SHADER_PROXY( "xxxx" ) xxxx;
		ORB_SHADER_PROXY( "xxxy" ) xxxy;
		ORB_SHADER_PROXY( "xxxz" ) xxxz;
		ORB_SHADER_PROXY( "xxxw" ) xxxw;
		ORB_SHADER_PROXY( "xxyx" ) xxyx;
		ORB_SHADER_PROXY( "xxyy" ) xxyy;
		ORB_SHADER_PROXY( "xxyz" ) xxyz;
		ORB_SHADER_PROXY( "xxyw" ) xxyw;
		ORB_SHADER_PROXY( "xxzx" ) xxzx;
		ORB_SHADER_PROXY( "xxzy" ) xxzy;
		ORB_SHADER_PROXY( "xxzz" ) xxzz;
		ORB_SHADER_PROXY( "xxzw" ) xxzw;
		ORB_SHADER_PROXY( "xxwx" ) xxwx;
		ORB_SHADER_PROXY( "xxwy" ) xxwy;
		ORB_SHADER_PROXY( "xxwz" ) xxwz;
		ORB_SHADER_PROXY( "xxww" ) xxww;
		ORB_SHADER_PROXY( "xyxx" ) xyxx;
		ORB_SHADER_PROXY( "xyxy" ) xyxy;
		ORB_SHADER_PROXY( "xyxz" ) xyxz;
		ORB_SHADER_PROXY( "xyxw" ) xyxw;
		ORB_SHADER_PROXY( "xyyx" ) xyyx;
		ORB_SHADER_PROXY( "xyyy" ) xyyy;
		ORB_SHADER_PROXY( "xyyz" ) xyyz;
		ORB_SHADER_PROXY( "xyyw" ) xyyw;
		ORB_SHADER_PROXY( "xyzx" ) xyzx;
		ORB_SHADER_PROXY( "xyzy" ) xyzy;
		ORB_SHADER_PROXY( "xyzz" ) xyzz;
		ORB_SHADER_PROXY( "xyzw" ) xyzw;
		ORB_SHADER_PROXY( "xywx" ) xywx;
		ORB_SHADER_PROXY( "xywy" ) xywy;
		ORB_SHADER_PROXY( "xywz" ) xywz;
		ORB_SHADER_PROXY( "xyww" ) xyww;
		ORB_SHADER_PROXY( "xzxx" ) xzxx;
		ORB_SHADER_PROXY( "xzxy" ) xzxy;
		ORB_SHADER_PROXY( "xzxz" ) xzxz;
		ORB_SHADER_PROXY( "xzxw" ) xzxw;
		ORB_SHADER_PROXY( "xzyx" ) xzyx;
		ORB_SHADER_PROXY( "xzyy" ) xzyy;
		ORB_SHADER_PROXY( "xzyz" ) xzyz;
		ORB_SHADER_PROXY( "xzyw" ) xzyw;
		ORB_SHADER_PROXY( "xzzx" ) xzzx;
		ORB_SHADER_PROXY( "xzzy" ) xzzy;
		ORB_SHADER_PROXY( "xzzz" ) xzzz;
		ORB_SHADER_PROXY( "xzzw" ) xzzw;
		ORB_SHADER_PROXY( "xzwx" ) xzwx;
		ORB_SHADER_PROXY( "xzwy" ) xzwy;
		ORB_SHADER_PROXY( "xzwz" ) xzwz;
		ORB_SHADER_PROXY( "xzww" ) xzww;
		ORB_SHADER_PROXY( "xwxx" ) xwxx;
		ORB_SHADER_PROXY( "xwxy" ) xwxy;
		ORB_SHADER_PROXY( "xwxz" ) xwxz;
		ORB_SHADER_PROXY( "xwxw" ) xwxw;
		ORB_SHADER_PROXY( "xwyx" ) xwyx;
		ORB_SHADER_PROXY( "xwyy" ) xwyy;
		ORB_SHADER_PROXY( "xwyz" ) xwyz;
		ORB_SHADER_PROXY( "xwyw" ) xwyw;
		ORB_SHADER_PROXY( "xwzx" ) xwzx;
		ORB_SHADER_PROXY( "xwzy" ) xwzy;
		ORB_SHADER_PROXY( "xwzz" ) xwzz;
		ORB_SHADER_PROXY( "xwzw" ) xwzw;
		ORB_SHADER_PROXY( "xwwx" ) xwwx;
		ORB_SHADER_PROXY( "xwwy" ) xwwy;
		ORB_SHADER_PROXY( "xwwz" ) xwwz;
		ORB_SHADER_PROXY( "xwww" ) xwww;
		ORB_SHADER_PROXY( "yxxx" ) yxxx;
		ORB_SHADER_PROXY( "yxxy" ) yxxy;
		ORB_SHADER_PROXY( "yxxz" ) yxxz;
		ORB_SHADER_PROXY( "yxxw" ) yxxw;
		ORB_SHADER_PROXY( "yxyx" ) yxyx;
		ORB_SHADER_PROXY( "yxyy" ) yxyy;
		ORB_SHADER_PROXY( "yxyz" ) yxyz;
		ORB_SHADER_PROXY( "yxyw" ) yxyw;
		ORB_SHADER_PROXY( "yxzx" ) yxzx;
		ORB_SHADER_PROXY( "yxzy" ) yxzy;
		ORB_SHADER_PROXY( "yxzz" ) yxzz;
		ORB_SHADER_PROXY( "yxzw" ) yxzw;
		ORB_SHADER_PROXY( "yxwx" ) yxwx;
		ORB_SHADER_PROXY( "yxwy" ) yxwy;
		ORB_SHADER_PROXY( "yxwz" ) yxwz;
		ORB_SHADER_PROXY( "yxww" ) yxww;
		ORB_SHADER_PROXY( "yyxx" ) yyxx;
		ORB_SHADER_PROXY( "yyxy" ) yyxy;
		ORB_SHADER_PROXY( "yyxz" ) yyxz;
		ORB_SHADER_PROXY( "yyxw" ) yyxw;
		ORB_SHADER_PROXY( "yyyx" ) yyyx;
		ORB_SHADER_PROXY( "yyyy" ) yyyy;
		ORB_SHADER_PROXY( "yyyz" ) yyyz;
		ORB_SHADER_PROXY( "yyyw" ) yyyw;
		ORB_SHADER_PROXY( "yyzx" ) yyzx;
		ORB_SHADER_PROXY( "yyzy" ) yyzy;
		ORB_SHADER_PROXY( "yyzz" ) yyzz;
		ORB_SHADER_PROXY( "yyzw" ) yyzw;
		ORB_SHADER_PROXY( "yywx" ) yywx;
		ORB_SHADER_PROXY( "yywy" ) yywy;
		ORB_SHADER_PROXY( "yywz" ) yywz;
		ORB_SHADER_PROXY( "yyww" ) yyww;
		ORB_SHADER_PROXY( "yzxx" ) yzxx;
		ORB_SHADER_PROXY( "yzxy" ) yzxy;
		ORB_SHADER_PROXY( "yzxz" ) yzxz;
		ORB_SHADER_PROXY( "yzxw" ) yzxw;
		ORB_SHADER_PROXY( "yzyx" ) yzyx;
		ORB_SHADER_PROXY( "yzyy" ) yzyy;
		ORB_SHADER_PROXY( "yzyz" ) yzyz;
		ORB_SHADER_PROXY( "yzyw" ) yzyw;
		ORB_SHADER_PROXY( "yzzx" ) yzzx;
		ORB_SHADER_PROXY( "yzzy" ) yzzy;
		ORB_SHADER_PROXY( "yzzz" ) yzzz;
		ORB_SHADER_PROXY( "yzzw" ) yzzw;
		ORB_SHADER_PROXY( "yzwx" ) yzwx;
		ORB_SHADER_PROXY( "yzwy" ) yzwy;
		ORB_SHADER_PROXY( "yzwz" ) yzwz;
		ORB_SHADER_PROXY( "yzww" ) yzww;
		ORB_SHADER_PROXY( "ywxx" ) ywxx;
		ORB_SHADER_PROXY( "ywxy" ) ywxy;
		ORB_SHADER_PROXY( "ywxz" ) ywxz;
		ORB_SHADER_PROXY( "ywxw" ) ywxw;
		ORB_SHADER_PROXY( "ywyx" ) ywyx;
		ORB_SHADER_PROXY( "ywyy" ) ywyy;
		ORB_SHADER_PROXY( "ywyz" ) ywyz;
		ORB_SHADER_PROXY( "ywyw" ) ywyw;
		ORB_SHADER_PROXY( "ywzx" ) ywzx;
		ORB_SHADER_PROXY( "ywzy" ) ywzy;
		ORB_SHADER_PROXY( "ywzz" ) ywzz;
		ORB_SHADER_PROXY( "ywzw" ) ywzw;
		ORB_SHADER_PROXY( "ywwx" ) ywwx;
		ORB_SHADER_PROXY( "ywwy" ) ywwy;
		ORB_SHADER_PROXY( "ywwz" ) ywwz;
		ORB_SHADER_PROXY( "ywww" ) ywww;
		ORB_SHADER_PROXY( "zxxx" ) zxxx;
		ORB_SHADER_PROXY( "zxxy" ) zxxy;
		ORB_SHADER_PROXY( "zxxz" ) zxxz;
		ORB_SHADER_PROXY( "zxxw" ) zxxw;
		ORB_SHADER_PROXY( "zxyx" ) zxyx;
		ORB_SHADER_PROXY( "zxyy" ) zxyy;
		ORB_SHADER_PROXY( "zxyz" ) zxyz;
		ORB_SHADER_PROXY( "zxyw" ) zxyw;
		ORB_SHADER_PROXY( "zxzx" ) zxzx;
		ORB_SHADER_PROXY( "zxzy" ) zxzy;
		ORB_SHADER_PROXY( "zxzz" ) zxzz;
		ORB_SHADER_PROXY( "zxzw" ) zxzw;
		ORB_SHADER_PROXY( "zxwx" ) zxwx;
		ORB_SHADER_PROXY( "zxwy" ) zxwy;
		ORB_SHADER_PROXY( "zxwz" ) zxwz;
		ORB_SHADER_PROXY( "zxww" ) zxww;
		ORB_SHADER_PROXY( "zyxx" ) zyxx;
		ORB_SHADER_PROXY( "zyxy" ) zyxy;
		ORB_SHADER_PROXY( "zyxz" ) zyxz;
		ORB_SHADER_PROXY( "zyxw" ) zyxw;
		ORB_SHADER_PROXY( "zyyx" ) zyyx;
		ORB_SHADER_PROXY( "zyyy" ) zyyy;
		ORB_SHADER_PROXY( "zyyz" ) zyyz;
		ORB_SHADER_PROXY( "zyyw" ) zyyw;
		ORB_SHADER_PROXY( "zyzx" ) zyzx;
		ORB_SHADER_PROXY( "zyzy" ) zyzy;
		ORB_SHADER_PROXY( "zyzz" ) zyzz;
		ORB_SHADER_PROXY( "zyzw" ) zyzw;
		ORB_SHADER_PROXY( "zywx" ) zywx;
		ORB_SHADER_PROXY( "zywy" ) zywy;
		ORB_SHADER_PROXY( "zywz" ) zywz;
		ORB_SHADER_PROXY( "zyww" ) zyww;
		ORB_SHADER_PROXY( "zzxx" ) zzxx;
		ORB_SHADER_PROXY( "zzxy" ) zzxy;
		ORB_SHADER_PROXY( "zzxz" ) zzxz;
		ORB_SHADER_PROXY( "zzxw" ) zzxw;
		ORB_SHADER_PROXY( "zzyx" ) zzyx;
		ORB_SHADER_PROXY( "zzyy" ) zzyy;
		ORB_SHADER_PROXY( "zzyz" ) zzyz;
		ORB_SHADER_PROXY( "zzyw" ) zzyw;
		ORB_SHADER_PROXY( "zzzx" ) zzzx;
		ORB_SHADER_PROXY( "zzzy" ) zzzy;
		ORB_SHADER_PROXY( "zzzz" ) zzzz;
		ORB_SHADER_PROXY( "zzzw" ) zzzw;
		ORB_SHADER_PROXY( "zzwx" ) zzwx;
		ORB_SHADER_PROXY( "zzwy" ) zzwy;
		ORB_SHADER_PROXY( "zzwz" ) zzwz;
		ORB_SHADER_PROXY( "zzww" ) zzww;
		ORB_SHADER_PROXY( "zwxx" ) zwxx;
		ORB_SHADER_PROXY( "zwxy" ) zwxy;
		ORB_SHADER_PROXY( "zwxz" ) zwxz;
		ORB_SHADER_PROXY( "zwxw" ) zwxw;
		ORB_SHADER_PROXY( "zwyx" ) zwyx;
		ORB_SHADER_PROXY( "zwyy" ) zwyy;
		ORB_SHADER_PROXY( "zwyz" ) zwyz;
		ORB_SHADER_PROXY( "zwyw" ) zwyw;
		ORB_SHADER_PROXY( "zwzx" ) zwzx;
		ORB_SHADER_PROXY( "zwzy" ) zwzy;
		ORB_SHADER_PROXY( "zwzz" ) zwzz;
		ORB_SHADER_PROXY( "zwzw" ) zwzw;
		ORB_SHADER_PROXY( "zwwx" ) zwwx;
		ORB_SHADER_PROXY( "zwwy" ) zwwy;
		ORB_SHADER_PROXY( "zwwz" ) zwwz;
		ORB_SHADER_PROXY( "zwww" ) zwww;
		ORB_SHADER_PROXY( "wxxx" ) wxxx;
		ORB_SHADER_PROXY( "wxxy" ) wxxy;
		ORB_SHADER_PROXY( "wxxz" ) wxxz;
		ORB_SHADER_PROXY( "wxxw" ) wxxw;
		ORB_SHADER_PROXY( "wxyx" ) wxyx;
		ORB_SHADER_PROXY( "wxyy" ) wxyy;
		ORB_SHADER_PROXY( "wxyz" ) wxyz;
		ORB_SHADER_PROXY( "wxyw" ) wxyw;
		ORB_SHADER_PROXY( "wxzx" ) wxzx;
		ORB_SHADER_PROXY( "wxzy" ) wxzy;
		ORB_SHADER_PROXY( "wxzz" ) wxzz;
		ORB_SHADER_PROXY( "wxzw" ) wxzw;
		ORB_SHADER_PROXY( "wxwx" ) wxwx;
		ORB_SHADER_PROXY( "wxwy" ) wxwy;
		ORB_SHADER_PROXY( "wxwz" ) wxwz;
		ORB_SHADER_PROXY( "wxww" ) wxww;
		ORB_SHADER_PROXY( "wyxx" ) wyxx;
		ORB_SHADER_PROXY( "wyxy" ) wyxy;
		ORB_SHADER_PROXY( "wyxz" ) wyxz;
		ORB_SHADER_PROXY( "wyxw" ) wyxw;
		ORB_SHADER_PROXY( "wyyx" ) wyyx;
		ORB_SHADER_PROXY( "wyyy" ) wyyy;
		ORB_SHADER_PROXY( "wyyz" ) wyyz;
		ORB_SHADER_PROXY( "wyyw" ) wyyw;
		ORB_SHADER_PROXY( "wyzx" ) wyzx;
		ORB_SHADER_PROXY( "wyzy" ) wyzy;
		ORB_SHADER_PROXY( "wyzz" ) wyzz;
		ORB_SHADER_PROXY( "wyzw" ) wyzw;
		ORB_SHADER_PROXY( "wywx" ) wywx;
		ORB_SHADER_PROXY( "wywy" ) wywy;
		ORB_SHADER_PROXY( "wywz" ) wywz;
		ORB_SHADER_PROXY( "wyww" ) wyww;
		ORB_SHADER_PROXY( "wzxx" ) wzxx;
		ORB_SHADER_PROXY( "wzxy" ) wzxy;
		ORB_SHADER_PROXY( "wzxz" ) wzxz;
		ORB_SHADER_PROXY( "wzxw" ) wzxw;
		ORB_SHADER_PROXY( "wzyx" ) wzyx;
		ORB_SHADER_PROXY( "wzyy" ) wzyy;
		ORB_SHADER_PROXY( "wzyz" ) wzyz;
		ORB_SHADER_PROXY( "wzyw" ) wzyw;
		ORB_SHADER_PROXY( "wzzx" ) wzzx;
		ORB_SHADER_PROXY( "wzzy" ) wzzy;
		ORB_SHADER_PROXY( "wzzz" ) wzzz;
		ORB_SHADER_PROXY( "wzzw" ) wzzw;
		ORB_SHADER_PROXY( "wzwx" ) wzwx;
		ORB_SHADER_PROXY( "wzwy" ) wzwy;
		ORB_SHADER_PROXY( "wzwz" ) wzwz;
		ORB_SHADER_PROXY( "wzww" ) wzww;
		ORB_SHADER_PROXY( "wwxx" ) wwxx;
		ORB_SHADER_PROXY( "wwxy" ) wwxy;
		ORB_SHADER_PROXY( "wwxz" ) wwxz;
		ORB_SHADER_PROXY( "wwxw" ) wwxw;
		ORB_SHADER_PROXY( "wwyx" ) wwyx;
		ORB_SHADER_PROXY( "wwyy" ) wwyy;
		ORB_SHADER_PROXY( "wwyz" ) wwyz;
		ORB_SHADER_PROXY( "wwyw" ) wwyw;
		ORB_SHADER_PROXY( "wwzx" ) wwzx;
		ORB_SHADER_PROXY( "wwzy" ) wwzy;
		ORB_SHADER_PROXY( "wwzz" ) wwzz;
		ORB_SHADER_PROXY( "wwzw" ) wwzw;
		ORB_SHADER_PROXY( "wwwx" ) wwwx;
		ORB_SHADER_PROXY( "wwwy" ) wwwy;
		ORB_SHADER_PROXY( "wwwz" ) wwwz;
		ORB_SHADER_PROXY( "wwww" ) wwww;

		ORB_SHADER_PROXY( "r" ) r;
		ORB_SHADER_PROXY( "g" ) g;
		ORB_SHADER_PROXY( "b" ) b;
		ORB_SHADER_PROXY( "a" ) a;

		ORB_SHADER_PROXY( "rr" ) rr;
		ORB_SHADER_PROXY( "rg" ) rg;
		ORB_SHADER_PROXY( "rb" ) rb;
		ORB_SHADER_PROXY( "ra" ) ra;
		ORB_SHADER_PROXY( "gr" ) gr;
		ORB_SHADER_PROXY( "gg" ) gg;
		ORB_SHADER_PROXY( "gb" ) gb;
		ORB_SHADER_PROXY( "ga" ) ga;
		ORB_SHADER_PROXY( "br" ) br;
		ORB_SHADER_PROXY( "bg" ) bg;
		ORB_SHADER_PROXY( "bb" ) bb;
		ORB_SHADER_PROXY( "ba" ) ba;
		ORB_SHADER_PROXY( "ar" ) ar;
		ORB_SHADER_PROXY( "ag" ) ag;
		ORB_SHADER_PROXY( "ab" ) ab;
		ORB_SHADER_PROXY( "aa" ) aa;

		ORB_SHADER_PROXY( "rrr" ) rrr;
		ORB_SHADER_PROXY( "rrg" ) rrg;
		ORB_SHADER_PROXY( "rrb" ) rrb;
		ORB_SHADER_PROXY( "rra" ) rra;
		ORB_SHADER_PROXY( "rgr" ) rgr;
		ORB_SHADER_PROXY( "rgg" ) rgg;
		ORB_SHADER_PROXY( "rgb" ) rgb;
		ORB_SHADER_PROXY( "rga" ) rga;
		ORB_SHADER_PROXY( "rbr" ) rbr;
		ORB_SHADER_PROXY( "rbg" ) rbg;
		ORB_SHADER_PROXY( "rbb" ) rbb;
		ORB_SHADER_PROXY( "rba" ) rba;
		ORB_SHADER_PROXY( "rar" ) rar;
		ORB_SHADER_PROXY( "rag" ) rag;
		ORB_SHADER_PROXY( "rab" ) rab;
		ORB_SHADER_PROXY( "raa" ) raa;
		ORB_SHADER_PROXY( "grr" ) grr;
		ORB_SHADER_PROXY( "grg" ) grg;
		ORB_SHADER_PROXY( "grb" ) grb;
		ORB_SHADER_PROXY( "gra" ) gra;
		ORB_SHADER_PROXY( "ggr" ) ggr;
		ORB_SHADER_PROXY( "ggg" ) ggg;
		ORB_SHADER_PROXY( "ggb" ) ggb;
		ORB_SHADER_PROXY( "gga" ) gga;
		ORB_SHADER_PROXY( "gbr" ) gbr;
		ORB_SHADER_PROXY( "gbg" ) gbg;
		ORB_SHADER_PROXY( "gbb" ) gbb;
		ORB_SHADER_PROXY( "gba" ) gba;
		ORB_SHADER_PROXY( "gar" ) gar;
		ORB_SHADER_PROXY( "gag" ) gag;
		ORB_SHADER_PROXY( "gab" ) gab;
		ORB_SHADER_PROXY( "gaa" ) gaa;
		ORB_SHADER_PROXY( "brr" ) brr;
		ORB_SHADER_PROXY( "brg" ) brg;
		ORB_SHADER_PROXY( "brb" ) brb;
		ORB_SHADER_PROXY( "bra" ) bra;
		ORB_SHADER_PROXY( "bgr" ) bgr;
		ORB_SHADER_PROXY( "bgg" ) bgg;
		ORB_SHADER_PROXY( "bgb" ) bgb;
		ORB_SHADER_PROXY( "bga" ) bga;
		ORB_SHADER_PROXY( "bbr" ) bbr;
		ORB_SHADER_PROXY( "bbg" ) bbg;
		ORB_SHADER_PROXY( "bbb" ) bbb;
		ORB_SHADER_PROXY( "bba" ) bba;
		ORB_SHADER_PROXY( "bar" ) bar;
		ORB_SHADER_PROXY( "bag" ) bag;
		ORB_SHADER_PROXY( "bab" ) bab;
		ORB_SHADER_PROXY( "baa" ) baa;
		ORB_SHADER_PROXY( "arr" ) arr;
		ORB_SHADER_PROXY( "arg" ) arg;
		ORB_SHADER_PROXY( "arb" ) arb;
		ORB_SHADER_PROXY( "ara" ) ara;
		ORB_SHADER_PROXY( "agr" ) agr;
		ORB_SHADER_PROXY( "agg" ) agg;
		ORB_SHADER_PROXY( "agb" ) agb;
		ORB_SHADER_PROXY( "aga" ) aga;
		ORB_SHADER_PROXY( "abr" ) abr;
		ORB_SHADER_PROXY( "abg" ) abg;
		ORB_SHADER_PROXY( "abb" ) abb;
		ORB_SHADER_PROXY( "aba" ) aba;
		ORB_SHADER_PROXY( "aar" ) aar;
		ORB_SHADER_PROXY( "aag" ) aag;
		ORB_SHADER_PROXY( "aab" ) aab;
		ORB_SHADER_PROXY( "aaa" ) aaa;

		ORB_SHADER_PROXY( "rrrr" ) rrrr;
		ORB_SHADER_PROXY( "rrrg" ) rrrg;
		ORB_SHADER_PROXY( "rrrb" ) rrrb;
		ORB_SHADER_PROXY( "rrra" ) rrra;
		ORB_SHADER_PROXY( "rrgr" ) rrgr;
		ORB_SHADER_PROXY( "rrgg" ) rrgg;
		ORB_SHADER_PROXY( "rrgb" ) rrgb;
		ORB_SHADER_PROXY( "rrga" ) rrga;
		ORB_SHADER_PROXY( "rrbr" ) rrbr;
		ORB_SHADER_PROXY( "rrbg" ) rrbg;
		ORB_SHADER_PROXY( "rrbb" ) rrbb;
		ORB_SHADER_PROXY( "rrba" ) rrba;
		ORB_SHADER_PROXY( "rrar" ) rrar;
		ORB_SHADER_PROXY( "rrag" ) rrag;
		ORB_SHADER_PROXY( "rrab" ) rrab;
		ORB_SHADER_PROXY( "rraa" ) rraa;
		ORB_SHADER_PROXY( "rgrr" ) rgrr;
		ORB_SHADER_PROXY( "rgrg" ) rgrg;
		ORB_SHADER_PROXY( "rgrb" ) rgrb;
		ORB_SHADER_PROXY( "rgra" ) rgra;
		ORB_SHADER_PROXY( "rggr" ) rggr;
		ORB_SHADER_PROXY( "rggg" ) rggg;
		ORB_SHADER_PROXY( "rggb" ) rggb;
		ORB_SHADER_PROXY( "rgga" ) rgga;
		ORB_SHADER_PROXY( "rgbr" ) rgbr;
		ORB_SHADER_PROXY( "rgbg" ) rgbg;
		ORB_SHADER_PROXY( "rgbb" ) rgbb;
		ORB_SHADER_PROXY( "rgba" ) rgba;
		ORB_SHADER_PROXY( "rgar" ) rgar;
		ORB_SHADER_PROXY( "rgag" ) rgag;
		ORB_SHADER_PROXY( "rgab" ) rgab;
		ORB_SHADER_PROXY( "rgaa" ) rgaa;
		ORB_SHADER_PROXY( "rbrr" ) rbrr;
		ORB_SHADER_PROXY( "rbrg" ) rbrg;
		ORB_SHADER_PROXY( "rbrb" ) rbrb;
		ORB_SHADER_PROXY( "rbra" ) rbra;
		ORB_SHADER_PROXY( "rbgr" ) rbgr;
		ORB_SHADER_PROXY( "rbgg" ) rbgg;
		ORB_SHADER_PROXY( "rbgb" ) rbgb;
		ORB_SHADER_PROXY( "rbga" ) rbga;
		ORB_SHADER_PROXY( "rbbr" ) rbbr;
		ORB_SHADER_PROXY( "rbbg" ) rbbg;
		ORB_SHADER_PROXY( "rbbb" ) rbbb;
		ORB_SHADER_PROXY( "rbba" ) rbba;
		ORB_SHADER_PROXY( "rbar" ) rbar;
		ORB_SHADER_PROXY( "rbag" ) rbag;
		ORB_SHADER_PROXY( "rbab" ) rbab;
		ORB_SHADER_PROXY( "rbaa" ) rbaa;
		ORB_SHADER_PROXY( "rarr" ) rarr;
		ORB_SHADER_PROXY( "rarg" ) rarg;
		ORB_SHADER_PROXY( "rarb" ) rarb;
		ORB_SHADER_PROXY( "rara" ) rara;
		ORB_SHADER_PROXY( "ragr" ) ragr;
		ORB_SHADER_PROXY( "ragg" ) ragg;
		ORB_SHADER_PROXY( "ragb" ) ragb;
		ORB_SHADER_PROXY( "raga" ) raga;
		ORB_SHADER_PROXY( "rabr" ) rabr;
		ORB_SHADER_PROXY( "rabg" ) rabg;
		ORB_SHADER_PROXY( "rabb" ) rabb;
		ORB_SHADER_PROXY( "raba" ) raba;
		ORB_SHADER_PROXY( "raar" ) raar;
		ORB_SHADER_PROXY( "raag" ) raag;
		ORB_SHADER_PROXY( "raab" ) raab;
		ORB_SHADER_PROXY( "raaa" ) raaa;
		ORB_SHADER_PROXY( "grrr" ) grrr;
		ORB_SHADER_PROXY( "grrg" ) grrg;
		ORB_SHADER_PROXY( "grrb" ) grrb;
		ORB_SHADER_PROXY( "grra" ) grra;
		ORB_SHADER_PROXY( "grgr" ) grgr;
		ORB_SHADER_PROXY( "grgg" ) grgg;
		ORB_SHADER_PROXY( "grgb" ) grgb;
		ORB_SHADER_PROXY( "grga" ) grga;
		ORB_SHADER_PROXY( "grbr" ) grbr;
		ORB_SHADER_PROXY( "grbg" ) grbg;
		ORB_SHADER_PROXY( "grbb" ) grbb;
		ORB_SHADER_PROXY( "grba" ) grba;
		ORB_SHADER_PROXY( "grar" ) grar;
		ORB_SHADER_PROXY( "grag" ) grag;
		ORB_SHADER_PROXY( "grab" ) grab;
		ORB_SHADER_PROXY( "graa" ) graa;
		ORB_SHADER_PROXY( "ggrr" ) ggrr;
		ORB_SHADER_PROXY( "ggrg" ) ggrg;
		ORB_SHADER_PROXY( "ggrb" ) ggrb;
		ORB_SHADER_PROXY( "ggra" ) ggra;
		ORB_SHADER_PROXY( "gggr" ) gggr;
		ORB_SHADER_PROXY( "gggg" ) gggg;
		ORB_SHADER_PROXY( "gggb" ) gggb;
		ORB_SHADER_PROXY( "ggga" ) ggga;
		ORB_SHADER_PROXY( "ggbr" ) ggbr;
		ORB_SHADER_PROXY( "ggbg" ) ggbg;
		ORB_SHADER_PROXY( "ggbb" ) ggbb;
		ORB_SHADER_PROXY( "ggba" ) ggba;
		ORB_SHADER_PROXY( "ggar" ) ggar;
		ORB_SHADER_PROXY( "ggag" ) ggag;
		ORB_SHADER_PROXY( "ggab" ) ggab;
		ORB_SHADER_PROXY( "ggaa" ) ggaa;
		ORB_SHADER_PROXY( "gbrr" ) gbrr;
		ORB_SHADER_PROXY( "gbrg" ) gbrg;
		ORB_SHADER_PROXY( "gbrb" ) gbrb;
		ORB_SHADER_PROXY( "gbra" ) gbra;
		ORB_SHADER_PROXY( "gbgr" ) gbgr;
		ORB_SHADER_PROXY( "gbgg" ) gbgg;
		ORB_SHADER_PROXY( "gbgb" ) gbgb;
		ORB_SHADER_PROXY( "gbga" ) gbga;
		ORB_SHADER_PROXY( "gbbr" ) gbbr;
		ORB_SHADER_PROXY( "gbbg" ) gbbg;
		ORB_SHADER_PROXY( "gbbb" ) gbbb;
		ORB_SHADER_PROXY( "gbba" ) gbba;
		ORB_SHADER_PROXY( "gbar" ) gbar;
		ORB_SHADER_PROXY( "gbag" ) gbag;
		ORB_SHADER_PROXY( "gbab" ) gbab;
		ORB_SHADER_PROXY( "gbaa" ) gbaa;
		ORB_SHADER_PROXY( "garr" ) garr;
		ORB_SHADER_PROXY( "garg" ) garg;
		ORB_SHADER_PROXY( "garb" ) garb;
		ORB_SHADER_PROXY( "gara" ) gara;
		ORB_SHADER_PROXY( "gagr" ) gagr;
		ORB_SHADER_PROXY( "gagg" ) gagg;
		ORB_SHADER_PROXY( "gagb" ) gagb;
		ORB_SHADER_PROXY( "gaga" ) gaga;
		ORB_SHADER_PROXY( "gabr" ) gabr;
		ORB_SHADER_PROXY( "gabg" ) gabg;
		ORB_SHADER_PROXY( "gabb" ) gabb;
		ORB_SHADER_PROXY( "gaba" ) gaba;
		ORB_SHADER_PROXY( "gaar" ) gaar;
		ORB_SHADER_PROXY( "gaag" ) gaag;
		ORB_SHADER_PROXY( "gaab" ) gaab;
		ORB_SHADER_PROXY( "gaaa" ) gaaa;
		ORB_SHADER_PROXY( "brrr" ) brrr;
		ORB_SHADER_PROXY( "brrg" ) brrg;
		ORB_SHADER_PROXY( "brrb" ) brrb;
		ORB_SHADER_PROXY( "brra" ) brra;
		ORB_SHADER_PROXY( "brgr" ) brgr;
		ORB_SHADER_PROXY( "brgg" ) brgg;
		ORB_SHADER_PROXY( "brgb" ) brgb;
		ORB_SHADER_PROXY( "brga" ) brga;
		ORB_SHADER_PROXY( "brbr" ) brbr;
		ORB_SHADER_PROXY( "brbg" ) brbg;
		ORB_SHADER_PROXY( "brbb" ) brbb;
		ORB_SHADER_PROXY( "brba" ) brba;
		ORB_SHADER_PROXY( "brar" ) brar;
		ORB_SHADER_PROXY( "brag" ) brag;
		ORB_SHADER_PROXY( "brab" ) brab;
		ORB_SHADER_PROXY( "braa" ) braa;
		ORB_SHADER_PROXY( "bgrr" ) bgrr;
		ORB_SHADER_PROXY( "bgrg" ) bgrg;
		ORB_SHADER_PROXY( "bgrb" ) bgrb;
		ORB_SHADER_PROXY( "bgra" ) bgra;
		ORB_SHADER_PROXY( "bggr" ) bggr;
		ORB_SHADER_PROXY( "bggg" ) bggg;
		ORB_SHADER_PROXY( "bggb" ) bggb;
		ORB_SHADER_PROXY( "bgga" ) bgga;
		ORB_SHADER_PROXY( "bgbr" ) bgbr;
		ORB_SHADER_PROXY( "bgbg" ) bgbg;
		ORB_SHADER_PROXY( "bgbb" ) bgbb;
		ORB_SHADER_PROXY( "bgba" ) bgba;
		ORB_SHADER_PROXY( "bgar" ) bgar;
		ORB_SHADER_PROXY( "bgag" ) bgag;
		ORB_SHADER_PROXY( "bgab" ) bgab;
		ORB_SHADER_PROXY( "bgaa" ) bgaa;
		ORB_SHADER_PROXY( "bbrr" ) bbrr;
		ORB_SHADER_PROXY( "bbrg" ) bbrg;
		ORB_SHADER_PROXY( "bbrb" ) bbrb;
		ORB_SHADER_PROXY( "bbra" ) bbra;
		ORB_SHADER_PROXY( "bbgr" ) bbgr;
		ORB_SHADER_PROXY( "bbgg" ) bbgg;
		ORB_SHADER_PROXY( "bbgb" ) bbgb;
		ORB_SHADER_PROXY( "bbga" ) bbga;
		ORB_SHADER_PROXY( "bbbr" ) bbbr;
		ORB_SHADER_PROXY( "bbbg" ) bbbg;
		ORB_SHADER_PROXY( "bbbb" ) bbbb;
		ORB_SHADER_PROXY( "bbba" ) bbba;
		ORB_SHADER_PROXY( "bbar" ) bbar;
		ORB_SHADER_PROXY( "bbag" ) bbag;
		ORB_SHADER_PROXY( "bbab" ) bbab;
		ORB_SHADER_PROXY( "bbaa" ) bbaa;
		ORB_SHADER_PROXY( "barr" ) barr;
		ORB_SHADER_PROXY( "barg" ) barg;
		ORB_SHADER_PROXY( "barb" ) barb;
		ORB_SHADER_PROXY( "bara" ) bara;
		ORB_SHADER_PROXY( "bagr" ) bagr;
		ORB_SHADER_PROXY( "bagg" ) bagg;
		ORB_SHADER_PROXY( "bagb" ) bagb;
		ORB_SHADER_PROXY( "baga" ) baga;
		ORB_SHADER_PROXY( "babr" ) babr;
		ORB_SHADER_PROXY( "babg" ) babg;
		ORB_SHADER_PROXY( "babb" ) babb;
		ORB_SHADER_PROXY( "baba" ) baba;
		ORB_SHADER_PROXY( "baar" ) baar;
		ORB_SHADER_PROXY( "baag" ) baag;
		ORB_SHADER_PROXY( "baab" ) baab;
		ORB_SHADER_PROXY( "baaa" ) baaa;
		ORB_SHADER_PROXY( "arrr" ) arrr;
		ORB_SHADER_PROXY( "arrg" ) arrg;
		ORB_SHADER_PROXY( "arrb" ) arrb;
		ORB_SHADER_PROXY( "arra" ) arra;
		ORB_SHADER_PROXY( "argr" ) argr;
		ORB_SHADER_PROXY( "argg" ) argg;
		ORB_SHADER_PROXY( "argb" ) argb;
		ORB_SHADER_PROXY( "arga" ) arga;
		ORB_SHADER_PROXY( "arbr" ) arbr;
		ORB_SHADER_PROXY( "arbg" ) arbg;
		ORB_SHADER_PROXY( "arbb" ) arbb;
		ORB_SHADER_PROXY( "arba" ) arba;
		ORB_SHADER_PROXY( "arar" ) arar;
		ORB_SHADER_PROXY( "arag" ) arag;
		ORB_SHADER_PROXY( "arab" ) arab;
		ORB_SHADER_PROXY( "araa" ) araa;
		ORB_SHADER_PROXY( "agrr" ) agrr;
		ORB_SHADER_PROXY( "agrg" ) agrg;
		ORB_SHADER_PROXY( "agrb" ) agrb;
		ORB_SHADER_PROXY( "agra" ) agra;
		ORB_SHADER_PROXY( "aggr" ) aggr;
		ORB_SHADER_PROXY( "aggg" ) aggg;
		ORB_SHADER_PROXY( "aggb" ) aggb;
		ORB_SHADER_PROXY( "agga" ) agga;
		ORB_SHADER_PROXY( "agbr" ) agbr;
		ORB_SHADER_PROXY( "agbg" ) agbg;
		ORB_SHADER_PROXY( "agbb" ) agbb;
		ORB_SHADER_PROXY( "agba" ) agba;
		ORB_SHADER_PROXY( "agar" ) agar;
		ORB_SHADER_PROXY( "agag" ) agag;
		ORB_SHADER_PROXY( "agab" ) agab;
		ORB_SHADER_PROXY( "agaa" ) agaa;
		ORB_SHADER_PROXY( "abrr" ) abrr;
		ORB_SHADER_PROXY( "abrg" ) abrg;
		ORB_SHADER_PROXY( "abrb" ) abrb;
		ORB_SHADER_PROXY( "abra" ) abra;
		ORB_SHADER_PROXY( "abgr" ) abgr;
		ORB_SHADER_PROXY( "abgg" ) abgg;
		ORB_SHADER_PROXY( "abgb" ) abgb;
		ORB_SHADER_PROXY( "abga" ) abga;
		ORB_SHADER_PROXY( "abbr" ) abbr;
		ORB_SHADER_PROXY( "abbg" ) abbg;
		ORB_SHADER_PROXY( "abbb" ) abbb;
		ORB_SHADER_PROXY( "abba" ) abba;
		ORB_SHADER_PROXY( "abar" ) abar;
		ORB_SHADER_PROXY( "abag" ) abag;
		ORB_SHADER_PROXY( "abab" ) abab;
		ORB_SHADER_PROXY( "abaa" ) abaa;
		ORB_SHADER_PROXY( "aarr" ) aarr;
		ORB_SHADER_PROXY( "aarg" ) aarg;
		ORB_SHADER_PROXY( "aarb" ) aarb;
		ORB_SHADER_PROXY( "aara" ) aara;
		ORB_SHADER_PROXY( "aagr" ) aagr;
		ORB_SHADER_PROXY( "aagg" ) aagg;
		ORB_SHADER_PROXY( "aagb" ) aagb;
		ORB_SHADER_PROXY( "aaga" ) aaga;
		ORB_SHADER_PROXY( "aabr" ) aabr;
		ORB_SHADER_PROXY( "aabg" ) aabg;
		ORB_SHADER_PROXY( "aabb" ) aabb;
		ORB_SHADER_PROXY( "aaba" ) aaba;
		ORB_SHADER_PROXY( "aaar" ) aaar;
		ORB_SHADER_PROXY( "aaag" ) aaag;
		ORB_SHADER_PROXY( "aaab" ) aaab;
		ORB_SHADER_PROXY( "aaaa" ) aaaa;

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

	public:

		std::string GetValue() const override;

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

	public:

		std::string GetValue() const override;

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
