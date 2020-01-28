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
			static_cast< Variable >( *this ) *= rhs;
		}

		operator Variable( void ) const
		{
			Variable* parent = Variable::s_latest_accessed_variable;
			Variable  proxy_variable( parent->m_value + "." + proxy_name.value, GetVariableType() );

			/* If parent is stored, then the proxy can be considered stored too.
			 * Otherwise, we'd not be able to manipulate the proxies within variables.
			 * `foo.rgb *= 0.5;` would become `vec3 local = foo.rgb; local *= 0.5;` and `foo` would
			 * be left unchanged. */
			proxy_variable.m_stored = parent->m_stored;

			return proxy_variable;
		}

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

		Variable  operator* ( const Variable& ) const;
		Variable  operator+ ( const Variable& ) const;
		Variable  operator- ( void )            const;

		Variable* operator->( void );
		void      operator= ( const Variable& );
		void      operator+=( const Variable& );
		void      operator*=( const Variable& );

	private:

		/* Stores the value in a local variable. Useful when we want to manipulate proxies within a
		 * variable, since `Vec2(1.0, 0.5).g *= 2.0;` is ill-behaved. */
		void StoreValue( void );

	private:

		static Variable* s_latest_accessed_variable;

		std::string  m_value;
		VariableType m_type   = VariableType::Unknown;
		bool         m_stored = false;
		mutable bool m_used   = false;

	public:

		static constexpr ORB_SHADER_PROXY( "x" ) x{ };
		static constexpr ORB_SHADER_PROXY( "y" ) y{ };
		static constexpr ORB_SHADER_PROXY( "z" ) z{ };
		static constexpr ORB_SHADER_PROXY( "w" ) w{ };

		static constexpr ORB_SHADER_PROXY( "xx" ) xx{ };
		static constexpr ORB_SHADER_PROXY( "xy" ) xy{ };
		static constexpr ORB_SHADER_PROXY( "xz" ) xz{ };
		static constexpr ORB_SHADER_PROXY( "xw" ) xw{ };
		static constexpr ORB_SHADER_PROXY( "yx" ) yx{ };
		static constexpr ORB_SHADER_PROXY( "yy" ) yy{ };
		static constexpr ORB_SHADER_PROXY( "yz" ) yz{ };
		static constexpr ORB_SHADER_PROXY( "yw" ) yw{ };
		static constexpr ORB_SHADER_PROXY( "zx" ) zx{ };
		static constexpr ORB_SHADER_PROXY( "zy" ) zy{ };
		static constexpr ORB_SHADER_PROXY( "zz" ) zz{ };
		static constexpr ORB_SHADER_PROXY( "zw" ) zw{ };
		static constexpr ORB_SHADER_PROXY( "wx" ) wx{ };
		static constexpr ORB_SHADER_PROXY( "wy" ) wy{ };
		static constexpr ORB_SHADER_PROXY( "wz" ) wz{ };
		static constexpr ORB_SHADER_PROXY( "ww" ) ww{ };

		static constexpr ORB_SHADER_PROXY( "xxx" ) xxx{ };
		static constexpr ORB_SHADER_PROXY( "xxy" ) xxy{ };
		static constexpr ORB_SHADER_PROXY( "xxz" ) xxz{ };
		static constexpr ORB_SHADER_PROXY( "xxw" ) xxw{ };
		static constexpr ORB_SHADER_PROXY( "xyx" ) xyx{ };
		static constexpr ORB_SHADER_PROXY( "xyy" ) xyy{ };
		static constexpr ORB_SHADER_PROXY( "xyz" ) xyz{ };
		static constexpr ORB_SHADER_PROXY( "xyw" ) xyw{ };
		static constexpr ORB_SHADER_PROXY( "xzx" ) xzx{ };
		static constexpr ORB_SHADER_PROXY( "xzy" ) xzy{ };
		static constexpr ORB_SHADER_PROXY( "xzz" ) xzz{ };
		static constexpr ORB_SHADER_PROXY( "xzw" ) xzw{ };
		static constexpr ORB_SHADER_PROXY( "xwx" ) xwx{ };
		static constexpr ORB_SHADER_PROXY( "xwy" ) xwy{ };
		static constexpr ORB_SHADER_PROXY( "xwz" ) xwz{ };
		static constexpr ORB_SHADER_PROXY( "xww" ) xww{ };
		static constexpr ORB_SHADER_PROXY( "yxx" ) yxx{ };
		static constexpr ORB_SHADER_PROXY( "yxy" ) yxy{ };
		static constexpr ORB_SHADER_PROXY( "yxz" ) yxz{ };
		static constexpr ORB_SHADER_PROXY( "yxw" ) yxw{ };
		static constexpr ORB_SHADER_PROXY( "yyx" ) yyx{ };
		static constexpr ORB_SHADER_PROXY( "yyy" ) yyy{ };
		static constexpr ORB_SHADER_PROXY( "yyz" ) yyz{ };
		static constexpr ORB_SHADER_PROXY( "yyw" ) yyw{ };
		static constexpr ORB_SHADER_PROXY( "yzx" ) yzx{ };
		static constexpr ORB_SHADER_PROXY( "yzy" ) yzy{ };
		static constexpr ORB_SHADER_PROXY( "yzz" ) yzz{ };
		static constexpr ORB_SHADER_PROXY( "yzw" ) yzw{ };
		static constexpr ORB_SHADER_PROXY( "ywx" ) ywx{ };
		static constexpr ORB_SHADER_PROXY( "ywy" ) ywy{ };
		static constexpr ORB_SHADER_PROXY( "ywz" ) ywz{ };
		static constexpr ORB_SHADER_PROXY( "yww" ) yww{ };
		static constexpr ORB_SHADER_PROXY( "zxx" ) zxx{ };
		static constexpr ORB_SHADER_PROXY( "zxy" ) zxy{ };
		static constexpr ORB_SHADER_PROXY( "zxz" ) zxz{ };
		static constexpr ORB_SHADER_PROXY( "zxw" ) zxw{ };
		static constexpr ORB_SHADER_PROXY( "zyx" ) zyx{ };
		static constexpr ORB_SHADER_PROXY( "zyy" ) zyy{ };
		static constexpr ORB_SHADER_PROXY( "zyz" ) zyz{ };
		static constexpr ORB_SHADER_PROXY( "zyw" ) zyw{ };
		static constexpr ORB_SHADER_PROXY( "zzx" ) zzx{ };
		static constexpr ORB_SHADER_PROXY( "zzy" ) zzy{ };
		static constexpr ORB_SHADER_PROXY( "zzz" ) zzz{ };
		static constexpr ORB_SHADER_PROXY( "zzw" ) zzw{ };
		static constexpr ORB_SHADER_PROXY( "zwx" ) zwx{ };
		static constexpr ORB_SHADER_PROXY( "zwy" ) zwy{ };
		static constexpr ORB_SHADER_PROXY( "zwz" ) zwz{ };
		static constexpr ORB_SHADER_PROXY( "zww" ) zww{ };
		static constexpr ORB_SHADER_PROXY( "wxx" ) wxx{ };
		static constexpr ORB_SHADER_PROXY( "wxy" ) wxy{ };
		static constexpr ORB_SHADER_PROXY( "wxz" ) wxz{ };
		static constexpr ORB_SHADER_PROXY( "wxw" ) wxw{ };
		static constexpr ORB_SHADER_PROXY( "wyx" ) wyx{ };
		static constexpr ORB_SHADER_PROXY( "wyy" ) wyy{ };
		static constexpr ORB_SHADER_PROXY( "wyz" ) wyz{ };
		static constexpr ORB_SHADER_PROXY( "wyw" ) wyw{ };
		static constexpr ORB_SHADER_PROXY( "wzx" ) wzx{ };
		static constexpr ORB_SHADER_PROXY( "wzy" ) wzy{ };
		static constexpr ORB_SHADER_PROXY( "wzz" ) wzz{ };
		static constexpr ORB_SHADER_PROXY( "wzw" ) wzw{ };
		static constexpr ORB_SHADER_PROXY( "wwx" ) wwx{ };
		static constexpr ORB_SHADER_PROXY( "wwy" ) wwy{ };
		static constexpr ORB_SHADER_PROXY( "wwz" ) wwz{ };
		static constexpr ORB_SHADER_PROXY( "www" ) www{ };

		static constexpr ORB_SHADER_PROXY( "xxxx" ) xxxx{ };
		static constexpr ORB_SHADER_PROXY( "xxxy" ) xxxy{ };
		static constexpr ORB_SHADER_PROXY( "xxxz" ) xxxz{ };
		static constexpr ORB_SHADER_PROXY( "xxxw" ) xxxw{ };
		static constexpr ORB_SHADER_PROXY( "xxyx" ) xxyx{ };
		static constexpr ORB_SHADER_PROXY( "xxyy" ) xxyy{ };
		static constexpr ORB_SHADER_PROXY( "xxyz" ) xxyz{ };
		static constexpr ORB_SHADER_PROXY( "xxyw" ) xxyw{ };
		static constexpr ORB_SHADER_PROXY( "xxzx" ) xxzx{ };
		static constexpr ORB_SHADER_PROXY( "xxzy" ) xxzy{ };
		static constexpr ORB_SHADER_PROXY( "xxzz" ) xxzz{ };
		static constexpr ORB_SHADER_PROXY( "xxzw" ) xxzw{ };
		static constexpr ORB_SHADER_PROXY( "xxwx" ) xxwx{ };
		static constexpr ORB_SHADER_PROXY( "xxwy" ) xxwy{ };
		static constexpr ORB_SHADER_PROXY( "xxwz" ) xxwz{ };
		static constexpr ORB_SHADER_PROXY( "xxww" ) xxww{ };
		static constexpr ORB_SHADER_PROXY( "xyxx" ) xyxx{ };
		static constexpr ORB_SHADER_PROXY( "xyxy" ) xyxy{ };
		static constexpr ORB_SHADER_PROXY( "xyxz" ) xyxz{ };
		static constexpr ORB_SHADER_PROXY( "xyxw" ) xyxw{ };
		static constexpr ORB_SHADER_PROXY( "xyyx" ) xyyx{ };
		static constexpr ORB_SHADER_PROXY( "xyyy" ) xyyy{ };
		static constexpr ORB_SHADER_PROXY( "xyyz" ) xyyz{ };
		static constexpr ORB_SHADER_PROXY( "xyyw" ) xyyw{ };
		static constexpr ORB_SHADER_PROXY( "xyzx" ) xyzx{ };
		static constexpr ORB_SHADER_PROXY( "xyzy" ) xyzy{ };
		static constexpr ORB_SHADER_PROXY( "xyzz" ) xyzz{ };
		static constexpr ORB_SHADER_PROXY( "xyzw" ) xyzw{ };
		static constexpr ORB_SHADER_PROXY( "xywx" ) xywx{ };
		static constexpr ORB_SHADER_PROXY( "xywy" ) xywy{ };
		static constexpr ORB_SHADER_PROXY( "xywz" ) xywz{ };
		static constexpr ORB_SHADER_PROXY( "xyww" ) xyww{ };
		static constexpr ORB_SHADER_PROXY( "xzxx" ) xzxx{ };
		static constexpr ORB_SHADER_PROXY( "xzxy" ) xzxy{ };
		static constexpr ORB_SHADER_PROXY( "xzxz" ) xzxz{ };
		static constexpr ORB_SHADER_PROXY( "xzxw" ) xzxw{ };
		static constexpr ORB_SHADER_PROXY( "xzyx" ) xzyx{ };
		static constexpr ORB_SHADER_PROXY( "xzyy" ) xzyy{ };
		static constexpr ORB_SHADER_PROXY( "xzyz" ) xzyz{ };
		static constexpr ORB_SHADER_PROXY( "xzyw" ) xzyw{ };
		static constexpr ORB_SHADER_PROXY( "xzzx" ) xzzx{ };
		static constexpr ORB_SHADER_PROXY( "xzzy" ) xzzy{ };
		static constexpr ORB_SHADER_PROXY( "xzzz" ) xzzz{ };
		static constexpr ORB_SHADER_PROXY( "xzzw" ) xzzw{ };
		static constexpr ORB_SHADER_PROXY( "xzwx" ) xzwx{ };
		static constexpr ORB_SHADER_PROXY( "xzwy" ) xzwy{ };
		static constexpr ORB_SHADER_PROXY( "xzwz" ) xzwz{ };
		static constexpr ORB_SHADER_PROXY( "xzww" ) xzww{ };
		static constexpr ORB_SHADER_PROXY( "xwxx" ) xwxx{ };
		static constexpr ORB_SHADER_PROXY( "xwxy" ) xwxy{ };
		static constexpr ORB_SHADER_PROXY( "xwxz" ) xwxz{ };
		static constexpr ORB_SHADER_PROXY( "xwxw" ) xwxw{ };
		static constexpr ORB_SHADER_PROXY( "xwyx" ) xwyx{ };
		static constexpr ORB_SHADER_PROXY( "xwyy" ) xwyy{ };
		static constexpr ORB_SHADER_PROXY( "xwyz" ) xwyz{ };
		static constexpr ORB_SHADER_PROXY( "xwyw" ) xwyw{ };
		static constexpr ORB_SHADER_PROXY( "xwzx" ) xwzx{ };
		static constexpr ORB_SHADER_PROXY( "xwzy" ) xwzy{ };
		static constexpr ORB_SHADER_PROXY( "xwzz" ) xwzz{ };
		static constexpr ORB_SHADER_PROXY( "xwzw" ) xwzw{ };
		static constexpr ORB_SHADER_PROXY( "xwwx" ) xwwx{ };
		static constexpr ORB_SHADER_PROXY( "xwwy" ) xwwy{ };
		static constexpr ORB_SHADER_PROXY( "xwwz" ) xwwz{ };
		static constexpr ORB_SHADER_PROXY( "xwww" ) xwww{ };
		static constexpr ORB_SHADER_PROXY( "yxxx" ) yxxx{ };
		static constexpr ORB_SHADER_PROXY( "yxxy" ) yxxy{ };
		static constexpr ORB_SHADER_PROXY( "yxxz" ) yxxz{ };
		static constexpr ORB_SHADER_PROXY( "yxxw" ) yxxw{ };
		static constexpr ORB_SHADER_PROXY( "yxyx" ) yxyx{ };
		static constexpr ORB_SHADER_PROXY( "yxyy" ) yxyy{ };
		static constexpr ORB_SHADER_PROXY( "yxyz" ) yxyz{ };
		static constexpr ORB_SHADER_PROXY( "yxyw" ) yxyw{ };
		static constexpr ORB_SHADER_PROXY( "yxzx" ) yxzx{ };
		static constexpr ORB_SHADER_PROXY( "yxzy" ) yxzy{ };
		static constexpr ORB_SHADER_PROXY( "yxzz" ) yxzz{ };
		static constexpr ORB_SHADER_PROXY( "yxzw" ) yxzw{ };
		static constexpr ORB_SHADER_PROXY( "yxwx" ) yxwx{ };
		static constexpr ORB_SHADER_PROXY( "yxwy" ) yxwy{ };
		static constexpr ORB_SHADER_PROXY( "yxwz" ) yxwz{ };
		static constexpr ORB_SHADER_PROXY( "yxww" ) yxww{ };
		static constexpr ORB_SHADER_PROXY( "yyxx" ) yyxx{ };
		static constexpr ORB_SHADER_PROXY( "yyxy" ) yyxy{ };
		static constexpr ORB_SHADER_PROXY( "yyxz" ) yyxz{ };
		static constexpr ORB_SHADER_PROXY( "yyxw" ) yyxw{ };
		static constexpr ORB_SHADER_PROXY( "yyyx" ) yyyx{ };
		static constexpr ORB_SHADER_PROXY( "yyyy" ) yyyy{ };
		static constexpr ORB_SHADER_PROXY( "yyyz" ) yyyz{ };
		static constexpr ORB_SHADER_PROXY( "yyyw" ) yyyw{ };
		static constexpr ORB_SHADER_PROXY( "yyzx" ) yyzx{ };
		static constexpr ORB_SHADER_PROXY( "yyzy" ) yyzy{ };
		static constexpr ORB_SHADER_PROXY( "yyzz" ) yyzz{ };
		static constexpr ORB_SHADER_PROXY( "yyzw" ) yyzw{ };
		static constexpr ORB_SHADER_PROXY( "yywx" ) yywx{ };
		static constexpr ORB_SHADER_PROXY( "yywy" ) yywy{ };
		static constexpr ORB_SHADER_PROXY( "yywz" ) yywz{ };
		static constexpr ORB_SHADER_PROXY( "yyww" ) yyww{ };
		static constexpr ORB_SHADER_PROXY( "yzxx" ) yzxx{ };
		static constexpr ORB_SHADER_PROXY( "yzxy" ) yzxy{ };
		static constexpr ORB_SHADER_PROXY( "yzxz" ) yzxz{ };
		static constexpr ORB_SHADER_PROXY( "yzxw" ) yzxw{ };
		static constexpr ORB_SHADER_PROXY( "yzyx" ) yzyx{ };
		static constexpr ORB_SHADER_PROXY( "yzyy" ) yzyy{ };
		static constexpr ORB_SHADER_PROXY( "yzyz" ) yzyz{ };
		static constexpr ORB_SHADER_PROXY( "yzyw" ) yzyw{ };
		static constexpr ORB_SHADER_PROXY( "yzzx" ) yzzx{ };
		static constexpr ORB_SHADER_PROXY( "yzzy" ) yzzy{ };
		static constexpr ORB_SHADER_PROXY( "yzzz" ) yzzz{ };
		static constexpr ORB_SHADER_PROXY( "yzzw" ) yzzw{ };
		static constexpr ORB_SHADER_PROXY( "yzwx" ) yzwx{ };
		static constexpr ORB_SHADER_PROXY( "yzwy" ) yzwy{ };
		static constexpr ORB_SHADER_PROXY( "yzwz" ) yzwz{ };
		static constexpr ORB_SHADER_PROXY( "yzww" ) yzww{ };
		static constexpr ORB_SHADER_PROXY( "ywxx" ) ywxx{ };
		static constexpr ORB_SHADER_PROXY( "ywxy" ) ywxy{ };
		static constexpr ORB_SHADER_PROXY( "ywxz" ) ywxz{ };
		static constexpr ORB_SHADER_PROXY( "ywxw" ) ywxw{ };
		static constexpr ORB_SHADER_PROXY( "ywyx" ) ywyx{ };
		static constexpr ORB_SHADER_PROXY( "ywyy" ) ywyy{ };
		static constexpr ORB_SHADER_PROXY( "ywyz" ) ywyz{ };
		static constexpr ORB_SHADER_PROXY( "ywyw" ) ywyw{ };
		static constexpr ORB_SHADER_PROXY( "ywzx" ) ywzx{ };
		static constexpr ORB_SHADER_PROXY( "ywzy" ) ywzy{ };
		static constexpr ORB_SHADER_PROXY( "ywzz" ) ywzz{ };
		static constexpr ORB_SHADER_PROXY( "ywzw" ) ywzw{ };
		static constexpr ORB_SHADER_PROXY( "ywwx" ) ywwx{ };
		static constexpr ORB_SHADER_PROXY( "ywwy" ) ywwy{ };
		static constexpr ORB_SHADER_PROXY( "ywwz" ) ywwz{ };
		static constexpr ORB_SHADER_PROXY( "ywww" ) ywww{ };
		static constexpr ORB_SHADER_PROXY( "zxxx" ) zxxx{ };
		static constexpr ORB_SHADER_PROXY( "zxxy" ) zxxy{ };
		static constexpr ORB_SHADER_PROXY( "zxxz" ) zxxz{ };
		static constexpr ORB_SHADER_PROXY( "zxxw" ) zxxw{ };
		static constexpr ORB_SHADER_PROXY( "zxyx" ) zxyx{ };
		static constexpr ORB_SHADER_PROXY( "zxyy" ) zxyy{ };
		static constexpr ORB_SHADER_PROXY( "zxyz" ) zxyz{ };
		static constexpr ORB_SHADER_PROXY( "zxyw" ) zxyw{ };
		static constexpr ORB_SHADER_PROXY( "zxzx" ) zxzx{ };
		static constexpr ORB_SHADER_PROXY( "zxzy" ) zxzy{ };
		static constexpr ORB_SHADER_PROXY( "zxzz" ) zxzz{ };
		static constexpr ORB_SHADER_PROXY( "zxzw" ) zxzw{ };
		static constexpr ORB_SHADER_PROXY( "zxwx" ) zxwx{ };
		static constexpr ORB_SHADER_PROXY( "zxwy" ) zxwy{ };
		static constexpr ORB_SHADER_PROXY( "zxwz" ) zxwz{ };
		static constexpr ORB_SHADER_PROXY( "zxww" ) zxww{ };
		static constexpr ORB_SHADER_PROXY( "zyxx" ) zyxx{ };
		static constexpr ORB_SHADER_PROXY( "zyxy" ) zyxy{ };
		static constexpr ORB_SHADER_PROXY( "zyxz" ) zyxz{ };
		static constexpr ORB_SHADER_PROXY( "zyxw" ) zyxw{ };
		static constexpr ORB_SHADER_PROXY( "zyyx" ) zyyx{ };
		static constexpr ORB_SHADER_PROXY( "zyyy" ) zyyy{ };
		static constexpr ORB_SHADER_PROXY( "zyyz" ) zyyz{ };
		static constexpr ORB_SHADER_PROXY( "zyyw" ) zyyw{ };
		static constexpr ORB_SHADER_PROXY( "zyzx" ) zyzx{ };
		static constexpr ORB_SHADER_PROXY( "zyzy" ) zyzy{ };
		static constexpr ORB_SHADER_PROXY( "zyzz" ) zyzz{ };
		static constexpr ORB_SHADER_PROXY( "zyzw" ) zyzw{ };
		static constexpr ORB_SHADER_PROXY( "zywx" ) zywx{ };
		static constexpr ORB_SHADER_PROXY( "zywy" ) zywy{ };
		static constexpr ORB_SHADER_PROXY( "zywz" ) zywz{ };
		static constexpr ORB_SHADER_PROXY( "zyww" ) zyww{ };
		static constexpr ORB_SHADER_PROXY( "zzxx" ) zzxx{ };
		static constexpr ORB_SHADER_PROXY( "zzxy" ) zzxy{ };
		static constexpr ORB_SHADER_PROXY( "zzxz" ) zzxz{ };
		static constexpr ORB_SHADER_PROXY( "zzxw" ) zzxw{ };
		static constexpr ORB_SHADER_PROXY( "zzyx" ) zzyx{ };
		static constexpr ORB_SHADER_PROXY( "zzyy" ) zzyy{ };
		static constexpr ORB_SHADER_PROXY( "zzyz" ) zzyz{ };
		static constexpr ORB_SHADER_PROXY( "zzyw" ) zzyw{ };
		static constexpr ORB_SHADER_PROXY( "zzzx" ) zzzx{ };
		static constexpr ORB_SHADER_PROXY( "zzzy" ) zzzy{ };
		static constexpr ORB_SHADER_PROXY( "zzzz" ) zzzz{ };
		static constexpr ORB_SHADER_PROXY( "zzzw" ) zzzw{ };
		static constexpr ORB_SHADER_PROXY( "zzwx" ) zzwx{ };
		static constexpr ORB_SHADER_PROXY( "zzwy" ) zzwy{ };
		static constexpr ORB_SHADER_PROXY( "zzwz" ) zzwz{ };
		static constexpr ORB_SHADER_PROXY( "zzww" ) zzww{ };
		static constexpr ORB_SHADER_PROXY( "zwxx" ) zwxx{ };
		static constexpr ORB_SHADER_PROXY( "zwxy" ) zwxy{ };
		static constexpr ORB_SHADER_PROXY( "zwxz" ) zwxz{ };
		static constexpr ORB_SHADER_PROXY( "zwxw" ) zwxw{ };
		static constexpr ORB_SHADER_PROXY( "zwyx" ) zwyx{ };
		static constexpr ORB_SHADER_PROXY( "zwyy" ) zwyy{ };
		static constexpr ORB_SHADER_PROXY( "zwyz" ) zwyz{ };
		static constexpr ORB_SHADER_PROXY( "zwyw" ) zwyw{ };
		static constexpr ORB_SHADER_PROXY( "zwzx" ) zwzx{ };
		static constexpr ORB_SHADER_PROXY( "zwzy" ) zwzy{ };
		static constexpr ORB_SHADER_PROXY( "zwzz" ) zwzz{ };
		static constexpr ORB_SHADER_PROXY( "zwzw" ) zwzw{ };
		static constexpr ORB_SHADER_PROXY( "zwwx" ) zwwx{ };
		static constexpr ORB_SHADER_PROXY( "zwwy" ) zwwy{ };
		static constexpr ORB_SHADER_PROXY( "zwwz" ) zwwz{ };
		static constexpr ORB_SHADER_PROXY( "zwww" ) zwww{ };
		static constexpr ORB_SHADER_PROXY( "wxxx" ) wxxx{ };
		static constexpr ORB_SHADER_PROXY( "wxxy" ) wxxy{ };
		static constexpr ORB_SHADER_PROXY( "wxxz" ) wxxz{ };
		static constexpr ORB_SHADER_PROXY( "wxxw" ) wxxw{ };
		static constexpr ORB_SHADER_PROXY( "wxyx" ) wxyx{ };
		static constexpr ORB_SHADER_PROXY( "wxyy" ) wxyy{ };
		static constexpr ORB_SHADER_PROXY( "wxyz" ) wxyz{ };
		static constexpr ORB_SHADER_PROXY( "wxyw" ) wxyw{ };
		static constexpr ORB_SHADER_PROXY( "wxzx" ) wxzx{ };
		static constexpr ORB_SHADER_PROXY( "wxzy" ) wxzy{ };
		static constexpr ORB_SHADER_PROXY( "wxzz" ) wxzz{ };
		static constexpr ORB_SHADER_PROXY( "wxzw" ) wxzw{ };
		static constexpr ORB_SHADER_PROXY( "wxwx" ) wxwx{ };
		static constexpr ORB_SHADER_PROXY( "wxwy" ) wxwy{ };
		static constexpr ORB_SHADER_PROXY( "wxwz" ) wxwz{ };
		static constexpr ORB_SHADER_PROXY( "wxww" ) wxww{ };
		static constexpr ORB_SHADER_PROXY( "wyxx" ) wyxx{ };
		static constexpr ORB_SHADER_PROXY( "wyxy" ) wyxy{ };
		static constexpr ORB_SHADER_PROXY( "wyxz" ) wyxz{ };
		static constexpr ORB_SHADER_PROXY( "wyxw" ) wyxw{ };
		static constexpr ORB_SHADER_PROXY( "wyyx" ) wyyx{ };
		static constexpr ORB_SHADER_PROXY( "wyyy" ) wyyy{ };
		static constexpr ORB_SHADER_PROXY( "wyyz" ) wyyz{ };
		static constexpr ORB_SHADER_PROXY( "wyyw" ) wyyw{ };
		static constexpr ORB_SHADER_PROXY( "wyzx" ) wyzx{ };
		static constexpr ORB_SHADER_PROXY( "wyzy" ) wyzy{ };
		static constexpr ORB_SHADER_PROXY( "wyzz" ) wyzz{ };
		static constexpr ORB_SHADER_PROXY( "wyzw" ) wyzw{ };
		static constexpr ORB_SHADER_PROXY( "wywx" ) wywx{ };
		static constexpr ORB_SHADER_PROXY( "wywy" ) wywy{ };
		static constexpr ORB_SHADER_PROXY( "wywz" ) wywz{ };
		static constexpr ORB_SHADER_PROXY( "wyww" ) wyww{ };
		static constexpr ORB_SHADER_PROXY( "wzxx" ) wzxx{ };
		static constexpr ORB_SHADER_PROXY( "wzxy" ) wzxy{ };
		static constexpr ORB_SHADER_PROXY( "wzxz" ) wzxz{ };
		static constexpr ORB_SHADER_PROXY( "wzxw" ) wzxw{ };
		static constexpr ORB_SHADER_PROXY( "wzyx" ) wzyx{ };
		static constexpr ORB_SHADER_PROXY( "wzyy" ) wzyy{ };
		static constexpr ORB_SHADER_PROXY( "wzyz" ) wzyz{ };
		static constexpr ORB_SHADER_PROXY( "wzyw" ) wzyw{ };
		static constexpr ORB_SHADER_PROXY( "wzzx" ) wzzx{ };
		static constexpr ORB_SHADER_PROXY( "wzzy" ) wzzy{ };
		static constexpr ORB_SHADER_PROXY( "wzzz" ) wzzz{ };
		static constexpr ORB_SHADER_PROXY( "wzzw" ) wzzw{ };
		static constexpr ORB_SHADER_PROXY( "wzwx" ) wzwx{ };
		static constexpr ORB_SHADER_PROXY( "wzwy" ) wzwy{ };
		static constexpr ORB_SHADER_PROXY( "wzwz" ) wzwz{ };
		static constexpr ORB_SHADER_PROXY( "wzww" ) wzww{ };
		static constexpr ORB_SHADER_PROXY( "wwxx" ) wwxx{ };
		static constexpr ORB_SHADER_PROXY( "wwxy" ) wwxy{ };
		static constexpr ORB_SHADER_PROXY( "wwxz" ) wwxz{ };
		static constexpr ORB_SHADER_PROXY( "wwxw" ) wwxw{ };
		static constexpr ORB_SHADER_PROXY( "wwyx" ) wwyx{ };
		static constexpr ORB_SHADER_PROXY( "wwyy" ) wwyy{ };
		static constexpr ORB_SHADER_PROXY( "wwyz" ) wwyz{ };
		static constexpr ORB_SHADER_PROXY( "wwyw" ) wwyw{ };
		static constexpr ORB_SHADER_PROXY( "wwzx" ) wwzx{ };
		static constexpr ORB_SHADER_PROXY( "wwzy" ) wwzy{ };
		static constexpr ORB_SHADER_PROXY( "wwzz" ) wwzz{ };
		static constexpr ORB_SHADER_PROXY( "wwzw" ) wwzw{ };
		static constexpr ORB_SHADER_PROXY( "wwwx" ) wwwx{ };
		static constexpr ORB_SHADER_PROXY( "wwwy" ) wwwy{ };
		static constexpr ORB_SHADER_PROXY( "wwwz" ) wwwz{ };
		static constexpr ORB_SHADER_PROXY( "wwww" ) wwww{ };

		static constexpr ORB_SHADER_PROXY( "r" ) r{ };
		static constexpr ORB_SHADER_PROXY( "g" ) g{ };
		static constexpr ORB_SHADER_PROXY( "b" ) b{ };
		static constexpr ORB_SHADER_PROXY( "a" ) a{ };

		static constexpr ORB_SHADER_PROXY( "rr" ) rr{ };
		static constexpr ORB_SHADER_PROXY( "rg" ) rg{ };
		static constexpr ORB_SHADER_PROXY( "rb" ) rb{ };
		static constexpr ORB_SHADER_PROXY( "ra" ) ra{ };
		static constexpr ORB_SHADER_PROXY( "gr" ) gr{ };
		static constexpr ORB_SHADER_PROXY( "gg" ) gg{ };
		static constexpr ORB_SHADER_PROXY( "gb" ) gb{ };
		static constexpr ORB_SHADER_PROXY( "ga" ) ga{ };
		static constexpr ORB_SHADER_PROXY( "br" ) br{ };
		static constexpr ORB_SHADER_PROXY( "bg" ) bg{ };
		static constexpr ORB_SHADER_PROXY( "bb" ) bb{ };
		static constexpr ORB_SHADER_PROXY( "ba" ) ba{ };
		static constexpr ORB_SHADER_PROXY( "ar" ) ar{ };
		static constexpr ORB_SHADER_PROXY( "ag" ) ag{ };
		static constexpr ORB_SHADER_PROXY( "ab" ) ab{ };
		static constexpr ORB_SHADER_PROXY( "aa" ) aa{ };

		static constexpr ORB_SHADER_PROXY( "rrr" ) rrr{ };
		static constexpr ORB_SHADER_PROXY( "rrg" ) rrg{ };
		static constexpr ORB_SHADER_PROXY( "rrb" ) rrb{ };
		static constexpr ORB_SHADER_PROXY( "rra" ) rra{ };
		static constexpr ORB_SHADER_PROXY( "rgr" ) rgr{ };
		static constexpr ORB_SHADER_PROXY( "rgg" ) rgg{ };
		static constexpr ORB_SHADER_PROXY( "rgb" ) rgb{ };
		static constexpr ORB_SHADER_PROXY( "rga" ) rga{ };
		static constexpr ORB_SHADER_PROXY( "rbr" ) rbr{ };
		static constexpr ORB_SHADER_PROXY( "rbg" ) rbg{ };
		static constexpr ORB_SHADER_PROXY( "rbb" ) rbb{ };
		static constexpr ORB_SHADER_PROXY( "rba" ) rba{ };
		static constexpr ORB_SHADER_PROXY( "rar" ) rar{ };
		static constexpr ORB_SHADER_PROXY( "rag" ) rag{ };
		static constexpr ORB_SHADER_PROXY( "rab" ) rab{ };
		static constexpr ORB_SHADER_PROXY( "raa" ) raa{ };
		static constexpr ORB_SHADER_PROXY( "grr" ) grr{ };
		static constexpr ORB_SHADER_PROXY( "grg" ) grg{ };
		static constexpr ORB_SHADER_PROXY( "grb" ) grb{ };
		static constexpr ORB_SHADER_PROXY( "gra" ) gra{ };
		static constexpr ORB_SHADER_PROXY( "ggr" ) ggr{ };
		static constexpr ORB_SHADER_PROXY( "ggg" ) ggg{ };
		static constexpr ORB_SHADER_PROXY( "ggb" ) ggb{ };
		static constexpr ORB_SHADER_PROXY( "gga" ) gga{ };
		static constexpr ORB_SHADER_PROXY( "gbr" ) gbr{ };
		static constexpr ORB_SHADER_PROXY( "gbg" ) gbg{ };
		static constexpr ORB_SHADER_PROXY( "gbb" ) gbb{ };
		static constexpr ORB_SHADER_PROXY( "gba" ) gba{ };
		static constexpr ORB_SHADER_PROXY( "gar" ) gar{ };
		static constexpr ORB_SHADER_PROXY( "gag" ) gag{ };
		static constexpr ORB_SHADER_PROXY( "gab" ) gab{ };
		static constexpr ORB_SHADER_PROXY( "gaa" ) gaa{ };
		static constexpr ORB_SHADER_PROXY( "brr" ) brr{ };
		static constexpr ORB_SHADER_PROXY( "brg" ) brg{ };
		static constexpr ORB_SHADER_PROXY( "brb" ) brb{ };
		static constexpr ORB_SHADER_PROXY( "bra" ) bra{ };
		static constexpr ORB_SHADER_PROXY( "bgr" ) bgr{ };
		static constexpr ORB_SHADER_PROXY( "bgg" ) bgg{ };
		static constexpr ORB_SHADER_PROXY( "bgb" ) bgb{ };
		static constexpr ORB_SHADER_PROXY( "bga" ) bga{ };
		static constexpr ORB_SHADER_PROXY( "bbr" ) bbr{ };
		static constexpr ORB_SHADER_PROXY( "bbg" ) bbg{ };
		static constexpr ORB_SHADER_PROXY( "bbb" ) bbb{ };
		static constexpr ORB_SHADER_PROXY( "bba" ) bba{ };
		static constexpr ORB_SHADER_PROXY( "bar" ) bar{ };
		static constexpr ORB_SHADER_PROXY( "bag" ) bag{ };
		static constexpr ORB_SHADER_PROXY( "bab" ) bab{ };
		static constexpr ORB_SHADER_PROXY( "baa" ) baa{ };
		static constexpr ORB_SHADER_PROXY( "arr" ) arr{ };
		static constexpr ORB_SHADER_PROXY( "arg" ) arg{ };
		static constexpr ORB_SHADER_PROXY( "arb" ) arb{ };
		static constexpr ORB_SHADER_PROXY( "ara" ) ara{ };
		static constexpr ORB_SHADER_PROXY( "agr" ) agr{ };
		static constexpr ORB_SHADER_PROXY( "agg" ) agg{ };
		static constexpr ORB_SHADER_PROXY( "agb" ) agb{ };
		static constexpr ORB_SHADER_PROXY( "aga" ) aga{ };
		static constexpr ORB_SHADER_PROXY( "abr" ) abr{ };
		static constexpr ORB_SHADER_PROXY( "abg" ) abg{ };
		static constexpr ORB_SHADER_PROXY( "abb" ) abb{ };
		static constexpr ORB_SHADER_PROXY( "aba" ) aba{ };
		static constexpr ORB_SHADER_PROXY( "aar" ) aar{ };
		static constexpr ORB_SHADER_PROXY( "aag" ) aag{ };
		static constexpr ORB_SHADER_PROXY( "aab" ) aab{ };
		static constexpr ORB_SHADER_PROXY( "aaa" ) aaa{ };

		static constexpr ORB_SHADER_PROXY( "rrrr" ) rrrr{ };
		static constexpr ORB_SHADER_PROXY( "rrrg" ) rrrg{ };
		static constexpr ORB_SHADER_PROXY( "rrrb" ) rrrb{ };
		static constexpr ORB_SHADER_PROXY( "rrra" ) rrra{ };
		static constexpr ORB_SHADER_PROXY( "rrgr" ) rrgr{ };
		static constexpr ORB_SHADER_PROXY( "rrgg" ) rrgg{ };
		static constexpr ORB_SHADER_PROXY( "rrgb" ) rrgb{ };
		static constexpr ORB_SHADER_PROXY( "rrga" ) rrga{ };
		static constexpr ORB_SHADER_PROXY( "rrbr" ) rrbr{ };
		static constexpr ORB_SHADER_PROXY( "rrbg" ) rrbg{ };
		static constexpr ORB_SHADER_PROXY( "rrbb" ) rrbb{ };
		static constexpr ORB_SHADER_PROXY( "rrba" ) rrba{ };
		static constexpr ORB_SHADER_PROXY( "rrar" ) rrar{ };
		static constexpr ORB_SHADER_PROXY( "rrag" ) rrag{ };
		static constexpr ORB_SHADER_PROXY( "rrab" ) rrab{ };
		static constexpr ORB_SHADER_PROXY( "rraa" ) rraa{ };
		static constexpr ORB_SHADER_PROXY( "rgrr" ) rgrr{ };
		static constexpr ORB_SHADER_PROXY( "rgrg" ) rgrg{ };
		static constexpr ORB_SHADER_PROXY( "rgrb" ) rgrb{ };
		static constexpr ORB_SHADER_PROXY( "rgra" ) rgra{ };
		static constexpr ORB_SHADER_PROXY( "rggr" ) rggr{ };
		static constexpr ORB_SHADER_PROXY( "rggg" ) rggg{ };
		static constexpr ORB_SHADER_PROXY( "rggb" ) rggb{ };
		static constexpr ORB_SHADER_PROXY( "rgga" ) rgga{ };
		static constexpr ORB_SHADER_PROXY( "rgbr" ) rgbr{ };
		static constexpr ORB_SHADER_PROXY( "rgbg" ) rgbg{ };
		static constexpr ORB_SHADER_PROXY( "rgbb" ) rgbb{ };
		static constexpr ORB_SHADER_PROXY( "rgba" ) rgba{ };
		static constexpr ORB_SHADER_PROXY( "rgar" ) rgar{ };
		static constexpr ORB_SHADER_PROXY( "rgag" ) rgag{ };
		static constexpr ORB_SHADER_PROXY( "rgab" ) rgab{ };
		static constexpr ORB_SHADER_PROXY( "rgaa" ) rgaa{ };
		static constexpr ORB_SHADER_PROXY( "rbrr" ) rbrr{ };
		static constexpr ORB_SHADER_PROXY( "rbrg" ) rbrg{ };
		static constexpr ORB_SHADER_PROXY( "rbrb" ) rbrb{ };
		static constexpr ORB_SHADER_PROXY( "rbra" ) rbra{ };
		static constexpr ORB_SHADER_PROXY( "rbgr" ) rbgr{ };
		static constexpr ORB_SHADER_PROXY( "rbgg" ) rbgg{ };
		static constexpr ORB_SHADER_PROXY( "rbgb" ) rbgb{ };
		static constexpr ORB_SHADER_PROXY( "rbga" ) rbga{ };
		static constexpr ORB_SHADER_PROXY( "rbbr" ) rbbr{ };
		static constexpr ORB_SHADER_PROXY( "rbbg" ) rbbg{ };
		static constexpr ORB_SHADER_PROXY( "rbbb" ) rbbb{ };
		static constexpr ORB_SHADER_PROXY( "rbba" ) rbba{ };
		static constexpr ORB_SHADER_PROXY( "rbar" ) rbar{ };
		static constexpr ORB_SHADER_PROXY( "rbag" ) rbag{ };
		static constexpr ORB_SHADER_PROXY( "rbab" ) rbab{ };
		static constexpr ORB_SHADER_PROXY( "rbaa" ) rbaa{ };
		static constexpr ORB_SHADER_PROXY( "rarr" ) rarr{ };
		static constexpr ORB_SHADER_PROXY( "rarg" ) rarg{ };
		static constexpr ORB_SHADER_PROXY( "rarb" ) rarb{ };
		static constexpr ORB_SHADER_PROXY( "rara" ) rara{ };
		static constexpr ORB_SHADER_PROXY( "ragr" ) ragr{ };
		static constexpr ORB_SHADER_PROXY( "ragg" ) ragg{ };
		static constexpr ORB_SHADER_PROXY( "ragb" ) ragb{ };
		static constexpr ORB_SHADER_PROXY( "raga" ) raga{ };
		static constexpr ORB_SHADER_PROXY( "rabr" ) rabr{ };
		static constexpr ORB_SHADER_PROXY( "rabg" ) rabg{ };
		static constexpr ORB_SHADER_PROXY( "rabb" ) rabb{ };
		static constexpr ORB_SHADER_PROXY( "raba" ) raba{ };
		static constexpr ORB_SHADER_PROXY( "raar" ) raar{ };
		static constexpr ORB_SHADER_PROXY( "raag" ) raag{ };
		static constexpr ORB_SHADER_PROXY( "raab" ) raab{ };
		static constexpr ORB_SHADER_PROXY( "raaa" ) raaa{ };
		static constexpr ORB_SHADER_PROXY( "grrr" ) grrr{ };
		static constexpr ORB_SHADER_PROXY( "grrg" ) grrg{ };
		static constexpr ORB_SHADER_PROXY( "grrb" ) grrb{ };
		static constexpr ORB_SHADER_PROXY( "grra" ) grra{ };
		static constexpr ORB_SHADER_PROXY( "grgr" ) grgr{ };
		static constexpr ORB_SHADER_PROXY( "grgg" ) grgg{ };
		static constexpr ORB_SHADER_PROXY( "grgb" ) grgb{ };
		static constexpr ORB_SHADER_PROXY( "grga" ) grga{ };
		static constexpr ORB_SHADER_PROXY( "grbr" ) grbr{ };
		static constexpr ORB_SHADER_PROXY( "grbg" ) grbg{ };
		static constexpr ORB_SHADER_PROXY( "grbb" ) grbb{ };
		static constexpr ORB_SHADER_PROXY( "grba" ) grba{ };
		static constexpr ORB_SHADER_PROXY( "grar" ) grar{ };
		static constexpr ORB_SHADER_PROXY( "grag" ) grag{ };
		static constexpr ORB_SHADER_PROXY( "grab" ) grab{ };
		static constexpr ORB_SHADER_PROXY( "graa" ) graa{ };
		static constexpr ORB_SHADER_PROXY( "ggrr" ) ggrr{ };
		static constexpr ORB_SHADER_PROXY( "ggrg" ) ggrg{ };
		static constexpr ORB_SHADER_PROXY( "ggrb" ) ggrb{ };
		static constexpr ORB_SHADER_PROXY( "ggra" ) ggra{ };
		static constexpr ORB_SHADER_PROXY( "gggr" ) gggr{ };
		static constexpr ORB_SHADER_PROXY( "gggg" ) gggg{ };
		static constexpr ORB_SHADER_PROXY( "gggb" ) gggb{ };
		static constexpr ORB_SHADER_PROXY( "ggga" ) ggga{ };
		static constexpr ORB_SHADER_PROXY( "ggbr" ) ggbr{ };
		static constexpr ORB_SHADER_PROXY( "ggbg" ) ggbg{ };
		static constexpr ORB_SHADER_PROXY( "ggbb" ) ggbb{ };
		static constexpr ORB_SHADER_PROXY( "ggba" ) ggba{ };
		static constexpr ORB_SHADER_PROXY( "ggar" ) ggar{ };
		static constexpr ORB_SHADER_PROXY( "ggag" ) ggag{ };
		static constexpr ORB_SHADER_PROXY( "ggab" ) ggab{ };
		static constexpr ORB_SHADER_PROXY( "ggaa" ) ggaa{ };
		static constexpr ORB_SHADER_PROXY( "gbrr" ) gbrr{ };
		static constexpr ORB_SHADER_PROXY( "gbrg" ) gbrg{ };
		static constexpr ORB_SHADER_PROXY( "gbrb" ) gbrb{ };
		static constexpr ORB_SHADER_PROXY( "gbra" ) gbra{ };
		static constexpr ORB_SHADER_PROXY( "gbgr" ) gbgr{ };
		static constexpr ORB_SHADER_PROXY( "gbgg" ) gbgg{ };
		static constexpr ORB_SHADER_PROXY( "gbgb" ) gbgb{ };
		static constexpr ORB_SHADER_PROXY( "gbga" ) gbga{ };
		static constexpr ORB_SHADER_PROXY( "gbbr" ) gbbr{ };
		static constexpr ORB_SHADER_PROXY( "gbbg" ) gbbg{ };
		static constexpr ORB_SHADER_PROXY( "gbbb" ) gbbb{ };
		static constexpr ORB_SHADER_PROXY( "gbba" ) gbba{ };
		static constexpr ORB_SHADER_PROXY( "gbar" ) gbar{ };
		static constexpr ORB_SHADER_PROXY( "gbag" ) gbag{ };
		static constexpr ORB_SHADER_PROXY( "gbab" ) gbab{ };
		static constexpr ORB_SHADER_PROXY( "gbaa" ) gbaa{ };
		static constexpr ORB_SHADER_PROXY( "garr" ) garr{ };
		static constexpr ORB_SHADER_PROXY( "garg" ) garg{ };
		static constexpr ORB_SHADER_PROXY( "garb" ) garb{ };
		static constexpr ORB_SHADER_PROXY( "gara" ) gara{ };
		static constexpr ORB_SHADER_PROXY( "gagr" ) gagr{ };
		static constexpr ORB_SHADER_PROXY( "gagg" ) gagg{ };
		static constexpr ORB_SHADER_PROXY( "gagb" ) gagb{ };
		static constexpr ORB_SHADER_PROXY( "gaga" ) gaga{ };
		static constexpr ORB_SHADER_PROXY( "gabr" ) gabr{ };
		static constexpr ORB_SHADER_PROXY( "gabg" ) gabg{ };
		static constexpr ORB_SHADER_PROXY( "gabb" ) gabb{ };
		static constexpr ORB_SHADER_PROXY( "gaba" ) gaba{ };
		static constexpr ORB_SHADER_PROXY( "gaar" ) gaar{ };
		static constexpr ORB_SHADER_PROXY( "gaag" ) gaag{ };
		static constexpr ORB_SHADER_PROXY( "gaab" ) gaab{ };
		static constexpr ORB_SHADER_PROXY( "gaaa" ) gaaa{ };
		static constexpr ORB_SHADER_PROXY( "brrr" ) brrr{ };
		static constexpr ORB_SHADER_PROXY( "brrg" ) brrg{ };
		static constexpr ORB_SHADER_PROXY( "brrb" ) brrb{ };
		static constexpr ORB_SHADER_PROXY( "brra" ) brra{ };
		static constexpr ORB_SHADER_PROXY( "brgr" ) brgr{ };
		static constexpr ORB_SHADER_PROXY( "brgg" ) brgg{ };
		static constexpr ORB_SHADER_PROXY( "brgb" ) brgb{ };
		static constexpr ORB_SHADER_PROXY( "brga" ) brga{ };
		static constexpr ORB_SHADER_PROXY( "brbr" ) brbr{ };
		static constexpr ORB_SHADER_PROXY( "brbg" ) brbg{ };
		static constexpr ORB_SHADER_PROXY( "brbb" ) brbb{ };
		static constexpr ORB_SHADER_PROXY( "brba" ) brba{ };
		static constexpr ORB_SHADER_PROXY( "brar" ) brar{ };
		static constexpr ORB_SHADER_PROXY( "brag" ) brag{ };
		static constexpr ORB_SHADER_PROXY( "brab" ) brab{ };
		static constexpr ORB_SHADER_PROXY( "braa" ) braa{ };
		static constexpr ORB_SHADER_PROXY( "bgrr" ) bgrr{ };
		static constexpr ORB_SHADER_PROXY( "bgrg" ) bgrg{ };
		static constexpr ORB_SHADER_PROXY( "bgrb" ) bgrb{ };
		static constexpr ORB_SHADER_PROXY( "bgra" ) bgra{ };
		static constexpr ORB_SHADER_PROXY( "bggr" ) bggr{ };
		static constexpr ORB_SHADER_PROXY( "bggg" ) bggg{ };
		static constexpr ORB_SHADER_PROXY( "bggb" ) bggb{ };
		static constexpr ORB_SHADER_PROXY( "bgga" ) bgga{ };
		static constexpr ORB_SHADER_PROXY( "bgbr" ) bgbr{ };
		static constexpr ORB_SHADER_PROXY( "bgbg" ) bgbg{ };
		static constexpr ORB_SHADER_PROXY( "bgbb" ) bgbb{ };
		static constexpr ORB_SHADER_PROXY( "bgba" ) bgba{ };
		static constexpr ORB_SHADER_PROXY( "bgar" ) bgar{ };
		static constexpr ORB_SHADER_PROXY( "bgag" ) bgag{ };
		static constexpr ORB_SHADER_PROXY( "bgab" ) bgab{ };
		static constexpr ORB_SHADER_PROXY( "bgaa" ) bgaa{ };
		static constexpr ORB_SHADER_PROXY( "bbrr" ) bbrr{ };
		static constexpr ORB_SHADER_PROXY( "bbrg" ) bbrg{ };
		static constexpr ORB_SHADER_PROXY( "bbrb" ) bbrb{ };
		static constexpr ORB_SHADER_PROXY( "bbra" ) bbra{ };
		static constexpr ORB_SHADER_PROXY( "bbgr" ) bbgr{ };
		static constexpr ORB_SHADER_PROXY( "bbgg" ) bbgg{ };
		static constexpr ORB_SHADER_PROXY( "bbgb" ) bbgb{ };
		static constexpr ORB_SHADER_PROXY( "bbga" ) bbga{ };
		static constexpr ORB_SHADER_PROXY( "bbbr" ) bbbr{ };
		static constexpr ORB_SHADER_PROXY( "bbbg" ) bbbg{ };
		static constexpr ORB_SHADER_PROXY( "bbbb" ) bbbb{ };
		static constexpr ORB_SHADER_PROXY( "bbba" ) bbba{ };
		static constexpr ORB_SHADER_PROXY( "bbar" ) bbar{ };
		static constexpr ORB_SHADER_PROXY( "bbag" ) bbag{ };
		static constexpr ORB_SHADER_PROXY( "bbab" ) bbab{ };
		static constexpr ORB_SHADER_PROXY( "bbaa" ) bbaa{ };
		static constexpr ORB_SHADER_PROXY( "barr" ) barr{ };
		static constexpr ORB_SHADER_PROXY( "barg" ) barg{ };
		static constexpr ORB_SHADER_PROXY( "barb" ) barb{ };
		static constexpr ORB_SHADER_PROXY( "bara" ) bara{ };
		static constexpr ORB_SHADER_PROXY( "bagr" ) bagr{ };
		static constexpr ORB_SHADER_PROXY( "bagg" ) bagg{ };
		static constexpr ORB_SHADER_PROXY( "bagb" ) bagb{ };
		static constexpr ORB_SHADER_PROXY( "baga" ) baga{ };
		static constexpr ORB_SHADER_PROXY( "babr" ) babr{ };
		static constexpr ORB_SHADER_PROXY( "babg" ) babg{ };
		static constexpr ORB_SHADER_PROXY( "babb" ) babb{ };
		static constexpr ORB_SHADER_PROXY( "baba" ) baba{ };
		static constexpr ORB_SHADER_PROXY( "baar" ) baar{ };
		static constexpr ORB_SHADER_PROXY( "baag" ) baag{ };
		static constexpr ORB_SHADER_PROXY( "baab" ) baab{ };
		static constexpr ORB_SHADER_PROXY( "baaa" ) baaa{ };
		static constexpr ORB_SHADER_PROXY( "arrr" ) arrr{ };
		static constexpr ORB_SHADER_PROXY( "arrg" ) arrg{ };
		static constexpr ORB_SHADER_PROXY( "arrb" ) arrb{ };
		static constexpr ORB_SHADER_PROXY( "arra" ) arra{ };
		static constexpr ORB_SHADER_PROXY( "argr" ) argr{ };
		static constexpr ORB_SHADER_PROXY( "argg" ) argg{ };
		static constexpr ORB_SHADER_PROXY( "argb" ) argb{ };
		static constexpr ORB_SHADER_PROXY( "arga" ) arga{ };
		static constexpr ORB_SHADER_PROXY( "arbr" ) arbr{ };
		static constexpr ORB_SHADER_PROXY( "arbg" ) arbg{ };
		static constexpr ORB_SHADER_PROXY( "arbb" ) arbb{ };
		static constexpr ORB_SHADER_PROXY( "arba" ) arba{ };
		static constexpr ORB_SHADER_PROXY( "arar" ) arar{ };
		static constexpr ORB_SHADER_PROXY( "arag" ) arag{ };
		static constexpr ORB_SHADER_PROXY( "arab" ) arab{ };
		static constexpr ORB_SHADER_PROXY( "araa" ) araa{ };
		static constexpr ORB_SHADER_PROXY( "agrr" ) agrr{ };
		static constexpr ORB_SHADER_PROXY( "agrg" ) agrg{ };
		static constexpr ORB_SHADER_PROXY( "agrb" ) agrb{ };
		static constexpr ORB_SHADER_PROXY( "agra" ) agra{ };
		static constexpr ORB_SHADER_PROXY( "aggr" ) aggr{ };
		static constexpr ORB_SHADER_PROXY( "aggg" ) aggg{ };
		static constexpr ORB_SHADER_PROXY( "aggb" ) aggb{ };
		static constexpr ORB_SHADER_PROXY( "agga" ) agga{ };
		static constexpr ORB_SHADER_PROXY( "agbr" ) agbr{ };
		static constexpr ORB_SHADER_PROXY( "agbg" ) agbg{ };
		static constexpr ORB_SHADER_PROXY( "agbb" ) agbb{ };
		static constexpr ORB_SHADER_PROXY( "agba" ) agba{ };
		static constexpr ORB_SHADER_PROXY( "agar" ) agar{ };
		static constexpr ORB_SHADER_PROXY( "agag" ) agag{ };
		static constexpr ORB_SHADER_PROXY( "agab" ) agab{ };
		static constexpr ORB_SHADER_PROXY( "agaa" ) agaa{ };
		static constexpr ORB_SHADER_PROXY( "abrr" ) abrr{ };
		static constexpr ORB_SHADER_PROXY( "abrg" ) abrg{ };
		static constexpr ORB_SHADER_PROXY( "abrb" ) abrb{ };
		static constexpr ORB_SHADER_PROXY( "abra" ) abra{ };
		static constexpr ORB_SHADER_PROXY( "abgr" ) abgr{ };
		static constexpr ORB_SHADER_PROXY( "abgg" ) abgg{ };
		static constexpr ORB_SHADER_PROXY( "abgb" ) abgb{ };
		static constexpr ORB_SHADER_PROXY( "abga" ) abga{ };
		static constexpr ORB_SHADER_PROXY( "abbr" ) abbr{ };
		static constexpr ORB_SHADER_PROXY( "abbg" ) abbg{ };
		static constexpr ORB_SHADER_PROXY( "abbb" ) abbb{ };
		static constexpr ORB_SHADER_PROXY( "abba" ) abba{ };
		static constexpr ORB_SHADER_PROXY( "abar" ) abar{ };
		static constexpr ORB_SHADER_PROXY( "abag" ) abag{ };
		static constexpr ORB_SHADER_PROXY( "abab" ) abab{ };
		static constexpr ORB_SHADER_PROXY( "abaa" ) abaa{ };
		static constexpr ORB_SHADER_PROXY( "aarr" ) aarr{ };
		static constexpr ORB_SHADER_PROXY( "aarg" ) aarg{ };
		static constexpr ORB_SHADER_PROXY( "aarb" ) aarb{ };
		static constexpr ORB_SHADER_PROXY( "aara" ) aara{ };
		static constexpr ORB_SHADER_PROXY( "aagr" ) aagr{ };
		static constexpr ORB_SHADER_PROXY( "aagg" ) aagg{ };
		static constexpr ORB_SHADER_PROXY( "aagb" ) aagb{ };
		static constexpr ORB_SHADER_PROXY( "aaga" ) aaga{ };
		static constexpr ORB_SHADER_PROXY( "aabr" ) aabr{ };
		static constexpr ORB_SHADER_PROXY( "aabg" ) aabg{ };
		static constexpr ORB_SHADER_PROXY( "aabb" ) aabb{ };
		static constexpr ORB_SHADER_PROXY( "aaba" ) aaba{ };
		static constexpr ORB_SHADER_PROXY( "aaar" ) aaar{ };
		static constexpr ORB_SHADER_PROXY( "aaag" ) aaag{ };
		static constexpr ORB_SHADER_PROXY( "aaab" ) aaab{ };
		static constexpr ORB_SHADER_PROXY( "aaaa" ) aaaa{ };

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
