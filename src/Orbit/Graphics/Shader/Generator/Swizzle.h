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
#include "Orbit/Graphics/Shader/Generator/Variables/IVariable.h"

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	class IVariable;

	template< char... Name >
	class SwizzleComponent
	{
	public:

		static constexpr StringLiteral< Name... > name{ };

	public:

		static constexpr DataType GetDataType( void )
		{
			switch( name.length )
			{
				case 1: { return DataType::Float; }
				case 2: { return DataType::FVec2; }
				case 3: { return DataType::FVec3; }
				case 4: { return DataType::FVec4; }
			}

			return DataType::Unknown;
		}

	public:

		IVariable operator+( const IVariable& rhs ) const
		{
			return ( static_cast< IVariable >( *this ) + rhs );
		}

		IVariable operator-( const IVariable& rhs ) const
		{
			return ( static_cast< IVariable >( *this ) - rhs );
		}

		IVariable operator*( const IVariable& rhs ) const
		{
			return ( static_cast< IVariable >( *this ) * rhs );
		}

		IVariable operator/( const IVariable& rhs ) const
		{
			return ( static_cast< IVariable >( *this ) / rhs );
		}

		void operator+=( const IVariable& rhs ) const
		{
			IVariable* parent = Swizzle::latest_accessed_variable;

			static_assert( !name.HasDuplicateChar(), "Cannot modify swizzles where the same component is used more than once" );

			parent->StoreValue();

			static_cast< IVariable >( *this ) += rhs;
		}

		void operator*=( const IVariable& rhs ) const
		{
			IVariable* parent = Swizzle::latest_accessed_variable;

			static_assert( !name.HasDuplicateChar(), "Cannot modify swizzles where the same component is used more than once" );

			parent->StoreValue();

			static_cast< IVariable >( *this ) *= rhs;
		}

		void operator=( const IVariable& rhs ) const
		{
			IVariable* parent = Swizzle::latest_accessed_variable;

			static_assert( !name.HasDuplicateChar(), "Cannot modify swizzles where the same component is used more than once" );

			parent->StoreValue();

			static_cast< IVariable >( *this ) = rhs;
		}

		operator IVariable( void ) const
		{
			IVariable* parent = Swizzle::latest_accessed_variable;
			IVariable  component_variable( parent->GetValue() + "." + name.value, GetDataType() );

			/* If parent is stored, then the swizzle component can be considered stored too.
			 * Otherwise, we'd not be able to manipulate the components within variables.
			 * `foo.rgb *= 0.5;` would become `vec3 local = foo.rgb; local *= 0.5;` and `foo` would
			 * be left unchanged. */
			if( parent->IsStored() )
				component_variable.SetStored();

			return component_variable;
		}

	};

#define ORB_SWIZZLE_COMPONENT( NAME )                    \
	decltype( ORB_NAMESPACE ShaderGen::SwizzleComponent< \
		ORB_NAMESPACE StringLiteralGrab< 0 >( NAME ),    \
		ORB_NAMESPACE StringLiteralGrab< 1 >( NAME ),    \
		ORB_NAMESPACE StringLiteralGrab< 2 >( NAME ),    \
		ORB_NAMESPACE StringLiteralGrab< 3 >( NAME ) >() )

	struct Swizzle
	{
		static ORB_API_GRAPHICS IVariable* latest_accessed_variable;

		static constexpr ORB_SWIZZLE_COMPONENT( "x" ) x{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "y" ) y{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "z" ) z{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "w" ) w{ };

		static constexpr ORB_SWIZZLE_COMPONENT( "xx" ) xx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xy" ) xy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xz" ) xz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xw" ) xw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yx" ) yx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yy" ) yy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yz" ) yz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yw" ) yw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zx" ) zx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zy" ) zy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zz" ) zz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zw" ) zw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wx" ) wx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wy" ) wy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wz" ) wz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ww" ) ww{ };

		static constexpr ORB_SWIZZLE_COMPONENT( "xxx" ) xxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxy" ) xxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxz" ) xxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxw" ) xxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyx" ) xyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyy" ) xyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyz" ) xyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyw" ) xyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzx" ) xzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzy" ) xzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzz" ) xzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzw" ) xzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwx" ) xwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwy" ) xwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwz" ) xwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xww" ) xww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxx" ) yxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxy" ) yxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxz" ) yxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxw" ) yxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyx" ) yyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyy" ) yyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyz" ) yyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyw" ) yyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzx" ) yzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzy" ) yzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzz" ) yzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzw" ) yzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywx" ) ywx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywy" ) ywy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywz" ) ywz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yww" ) yww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxx" ) zxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxy" ) zxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxz" ) zxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxw" ) zxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyx" ) zyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyy" ) zyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyz" ) zyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyw" ) zyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzx" ) zzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzy" ) zzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzz" ) zzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzw" ) zzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwx" ) zwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwy" ) zwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwz" ) zwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zww" ) zww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxx" ) wxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxy" ) wxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxz" ) wxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxw" ) wxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyx" ) wyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyy" ) wyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyz" ) wyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyw" ) wyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzx" ) wzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzy" ) wzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzz" ) wzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzw" ) wzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwx" ) wwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwy" ) wwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwz" ) wwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "www" ) www{ };

		static constexpr ORB_SWIZZLE_COMPONENT( "xxxx" ) xxxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxxy" ) xxxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxxz" ) xxxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxxw" ) xxxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxyx" ) xxyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxyy" ) xxyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxyz" ) xxyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxyw" ) xxyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxzx" ) xxzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxzy" ) xxzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxzz" ) xxzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxzw" ) xxzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxwx" ) xxwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxwy" ) xxwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxwz" ) xxwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xxww" ) xxww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyxx" ) xyxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyxy" ) xyxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyxz" ) xyxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyxw" ) xyxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyyx" ) xyyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyyy" ) xyyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyyz" ) xyyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyyw" ) xyyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyzx" ) xyzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyzy" ) xyzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyzz" ) xyzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyzw" ) xyzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xywx" ) xywx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xywy" ) xywy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xywz" ) xywz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xyww" ) xyww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzxx" ) xzxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzxy" ) xzxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzxz" ) xzxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzxw" ) xzxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzyx" ) xzyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzyy" ) xzyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzyz" ) xzyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzyw" ) xzyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzzx" ) xzzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzzy" ) xzzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzzz" ) xzzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzzw" ) xzzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzwx" ) xzwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzwy" ) xzwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzwz" ) xzwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xzww" ) xzww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwxx" ) xwxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwxy" ) xwxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwxz" ) xwxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwxw" ) xwxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwyx" ) xwyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwyy" ) xwyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwyz" ) xwyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwyw" ) xwyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwzx" ) xwzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwzy" ) xwzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwzz" ) xwzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwzw" ) xwzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwwx" ) xwwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwwy" ) xwwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwwz" ) xwwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "xwww" ) xwww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxxx" ) yxxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxxy" ) yxxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxxz" ) yxxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxxw" ) yxxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxyx" ) yxyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxyy" ) yxyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxyz" ) yxyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxyw" ) yxyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxzx" ) yxzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxzy" ) yxzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxzz" ) yxzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxzw" ) yxzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxwx" ) yxwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxwy" ) yxwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxwz" ) yxwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yxww" ) yxww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyxx" ) yyxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyxy" ) yyxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyxz" ) yyxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyxw" ) yyxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyyx" ) yyyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyyy" ) yyyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyyz" ) yyyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyyw" ) yyyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyzx" ) yyzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyzy" ) yyzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyzz" ) yyzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyzw" ) yyzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yywx" ) yywx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yywy" ) yywy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yywz" ) yywz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yyww" ) yyww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzxx" ) yzxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzxy" ) yzxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzxz" ) yzxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzxw" ) yzxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzyx" ) yzyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzyy" ) yzyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzyz" ) yzyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzyw" ) yzyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzzx" ) yzzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzzy" ) yzzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzzz" ) yzzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzzw" ) yzzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzwx" ) yzwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzwy" ) yzwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzwz" ) yzwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "yzww" ) yzww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywxx" ) ywxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywxy" ) ywxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywxz" ) ywxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywxw" ) ywxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywyx" ) ywyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywyy" ) ywyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywyz" ) ywyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywyw" ) ywyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywzx" ) ywzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywzy" ) ywzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywzz" ) ywzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywzw" ) ywzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywwx" ) ywwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywwy" ) ywwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywwz" ) ywwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ywww" ) ywww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxxx" ) zxxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxxy" ) zxxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxxz" ) zxxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxxw" ) zxxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxyx" ) zxyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxyy" ) zxyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxyz" ) zxyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxyw" ) zxyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxzx" ) zxzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxzy" ) zxzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxzz" ) zxzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxzw" ) zxzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxwx" ) zxwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxwy" ) zxwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxwz" ) zxwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zxww" ) zxww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyxx" ) zyxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyxy" ) zyxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyxz" ) zyxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyxw" ) zyxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyyx" ) zyyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyyy" ) zyyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyyz" ) zyyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyyw" ) zyyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyzx" ) zyzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyzy" ) zyzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyzz" ) zyzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyzw" ) zyzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zywx" ) zywx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zywy" ) zywy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zywz" ) zywz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zyww" ) zyww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzxx" ) zzxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzxy" ) zzxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzxz" ) zzxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzxw" ) zzxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzyx" ) zzyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzyy" ) zzyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzyz" ) zzyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzyw" ) zzyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzzx" ) zzzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzzy" ) zzzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzzz" ) zzzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzzw" ) zzzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzwx" ) zzwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzwy" ) zzwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzwz" ) zzwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zzww" ) zzww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwxx" ) zwxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwxy" ) zwxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwxz" ) zwxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwxw" ) zwxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwyx" ) zwyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwyy" ) zwyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwyz" ) zwyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwyw" ) zwyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwzx" ) zwzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwzy" ) zwzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwzz" ) zwzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwzw" ) zwzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwwx" ) zwwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwwy" ) zwwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwwz" ) zwwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "zwww" ) zwww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxxx" ) wxxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxxy" ) wxxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxxz" ) wxxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxxw" ) wxxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxyx" ) wxyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxyy" ) wxyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxyz" ) wxyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxyw" ) wxyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxzx" ) wxzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxzy" ) wxzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxzz" ) wxzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxzw" ) wxzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxwx" ) wxwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxwy" ) wxwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxwz" ) wxwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wxww" ) wxww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyxx" ) wyxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyxy" ) wyxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyxz" ) wyxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyxw" ) wyxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyyx" ) wyyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyyy" ) wyyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyyz" ) wyyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyyw" ) wyyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyzx" ) wyzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyzy" ) wyzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyzz" ) wyzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyzw" ) wyzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wywx" ) wywx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wywy" ) wywy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wywz" ) wywz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wyww" ) wyww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzxx" ) wzxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzxy" ) wzxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzxz" ) wzxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzxw" ) wzxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzyx" ) wzyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzyy" ) wzyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzyz" ) wzyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzyw" ) wzyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzzx" ) wzzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzzy" ) wzzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzzz" ) wzzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzzw" ) wzzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzwx" ) wzwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzwy" ) wzwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzwz" ) wzwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wzww" ) wzww{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwxx" ) wwxx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwxy" ) wwxy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwxz" ) wwxz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwxw" ) wwxw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwyx" ) wwyx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwyy" ) wwyy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwyz" ) wwyz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwyw" ) wwyw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwzx" ) wwzx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwzy" ) wwzy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwzz" ) wwzz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwzw" ) wwzw{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwwx" ) wwwx{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwwy" ) wwwy{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwwz" ) wwwz{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "wwww" ) wwww{ };

		static constexpr ORB_SWIZZLE_COMPONENT( "r" ) r{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "g" ) g{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "b" ) b{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "a" ) a{ };

		static constexpr ORB_SWIZZLE_COMPONENT( "rr" ) rr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rg" ) rg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rb" ) rb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ra" ) ra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gr" ) gr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gg" ) gg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gb" ) gb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ga" ) ga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "br" ) br{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bg" ) bg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bb" ) bb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ba" ) ba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ar" ) ar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ag" ) ag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ab" ) ab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aa" ) aa{ };

		static constexpr ORB_SWIZZLE_COMPONENT( "rrr" ) rrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrg" ) rrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrb" ) rrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rra" ) rra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgr" ) rgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgg" ) rgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgb" ) rgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rga" ) rga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbr" ) rbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbg" ) rbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbb" ) rbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rba" ) rba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rar" ) rar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rag" ) rag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rab" ) rab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "raa" ) raa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grr" ) grr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grg" ) grg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grb" ) grb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gra" ) gra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggr" ) ggr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggg" ) ggg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggb" ) ggb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gga" ) gga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbr" ) gbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbg" ) gbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbb" ) gbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gba" ) gba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gar" ) gar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gag" ) gag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gab" ) gab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gaa" ) gaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brr" ) brr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brg" ) brg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brb" ) brb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bra" ) bra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgr" ) bgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgg" ) bgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgb" ) bgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bga" ) bga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbr" ) bbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbg" ) bbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbb" ) bbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bba" ) bba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bar" ) bar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bag" ) bag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bab" ) bab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "baa" ) baa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arr" ) arr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arg" ) arg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arb" ) arb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ara" ) ara{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agr" ) agr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agg" ) agg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agb" ) agb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aga" ) aga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abr" ) abr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abg" ) abg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abb" ) abb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aba" ) aba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aar" ) aar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aag" ) aag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aab" ) aab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aaa" ) aaa{ };

		static constexpr ORB_SWIZZLE_COMPONENT( "rrrr" ) rrrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrrg" ) rrrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrrb" ) rrrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrra" ) rrra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrgr" ) rrgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrgg" ) rrgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrgb" ) rrgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrga" ) rrga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrbr" ) rrbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrbg" ) rrbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrbb" ) rrbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrba" ) rrba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrar" ) rrar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrag" ) rrag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rrab" ) rrab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rraa" ) rraa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgrr" ) rgrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgrg" ) rgrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgrb" ) rgrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgra" ) rgra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rggr" ) rggr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rggg" ) rggg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rggb" ) rggb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgga" ) rgga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgbr" ) rgbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgbg" ) rgbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgbb" ) rgbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgba" ) rgba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgar" ) rgar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgag" ) rgag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgab" ) rgab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rgaa" ) rgaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbrr" ) rbrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbrg" ) rbrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbrb" ) rbrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbra" ) rbra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbgr" ) rbgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbgg" ) rbgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbgb" ) rbgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbga" ) rbga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbbr" ) rbbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbbg" ) rbbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbbb" ) rbbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbba" ) rbba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbar" ) rbar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbag" ) rbag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbab" ) rbab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rbaa" ) rbaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rarr" ) rarr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rarg" ) rarg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rarb" ) rarb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rara" ) rara{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ragr" ) ragr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ragg" ) ragg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ragb" ) ragb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "raga" ) raga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rabr" ) rabr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rabg" ) rabg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "rabb" ) rabb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "raba" ) raba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "raar" ) raar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "raag" ) raag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "raab" ) raab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "raaa" ) raaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grrr" ) grrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grrg" ) grrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grrb" ) grrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grra" ) grra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grgr" ) grgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grgg" ) grgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grgb" ) grgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grga" ) grga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grbr" ) grbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grbg" ) grbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grbb" ) grbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grba" ) grba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grar" ) grar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grag" ) grag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "grab" ) grab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "graa" ) graa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggrr" ) ggrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggrg" ) ggrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggrb" ) ggrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggra" ) ggra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gggr" ) gggr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gggg" ) gggg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gggb" ) gggb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggga" ) ggga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggbr" ) ggbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggbg" ) ggbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggbb" ) ggbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggba" ) ggba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggar" ) ggar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggag" ) ggag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggab" ) ggab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "ggaa" ) ggaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbrr" ) gbrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbrg" ) gbrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbrb" ) gbrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbra" ) gbra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbgr" ) gbgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbgg" ) gbgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbgb" ) gbgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbga" ) gbga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbbr" ) gbbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbbg" ) gbbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbbb" ) gbbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbba" ) gbba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbar" ) gbar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbag" ) gbag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbab" ) gbab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gbaa" ) gbaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "garr" ) garr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "garg" ) garg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "garb" ) garb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gara" ) gara{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gagr" ) gagr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gagg" ) gagg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gagb" ) gagb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gaga" ) gaga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gabr" ) gabr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gabg" ) gabg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gabb" ) gabb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gaba" ) gaba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gaar" ) gaar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gaag" ) gaag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gaab" ) gaab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "gaaa" ) gaaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brrr" ) brrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brrg" ) brrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brrb" ) brrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brra" ) brra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brgr" ) brgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brgg" ) brgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brgb" ) brgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brga" ) brga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brbr" ) brbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brbg" ) brbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brbb" ) brbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brba" ) brba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brar" ) brar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brag" ) brag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "brab" ) brab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "braa" ) braa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgrr" ) bgrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgrg" ) bgrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgrb" ) bgrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgra" ) bgra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bggr" ) bggr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bggg" ) bggg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bggb" ) bggb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgga" ) bgga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgbr" ) bgbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgbg" ) bgbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgbb" ) bgbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgba" ) bgba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgar" ) bgar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgag" ) bgag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgab" ) bgab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bgaa" ) bgaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbrr" ) bbrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbrg" ) bbrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbrb" ) bbrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbra" ) bbra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbgr" ) bbgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbgg" ) bbgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbgb" ) bbgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbga" ) bbga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbbr" ) bbbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbbg" ) bbbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbbb" ) bbbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbba" ) bbba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbar" ) bbar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbag" ) bbag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbab" ) bbab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bbaa" ) bbaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "barr" ) barr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "barg" ) barg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "barb" ) barb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bara" ) bara{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bagr" ) bagr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bagg" ) bagg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "bagb" ) bagb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "baga" ) baga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "babr" ) babr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "babg" ) babg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "babb" ) babb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "baba" ) baba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "baar" ) baar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "baag" ) baag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "baab" ) baab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "baaa" ) baaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arrr" ) arrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arrg" ) arrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arrb" ) arrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arra" ) arra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "argr" ) argr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "argg" ) argg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "argb" ) argb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arga" ) arga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arbr" ) arbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arbg" ) arbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arbb" ) arbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arba" ) arba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arar" ) arar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arag" ) arag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "arab" ) arab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "araa" ) araa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agrr" ) agrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agrg" ) agrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agrb" ) agrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agra" ) agra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aggr" ) aggr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aggg" ) aggg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aggb" ) aggb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agga" ) agga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agbr" ) agbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agbg" ) agbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agbb" ) agbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agba" ) agba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agar" ) agar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agag" ) agag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agab" ) agab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "agaa" ) agaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abrr" ) abrr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abrg" ) abrg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abrb" ) abrb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abra" ) abra{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abgr" ) abgr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abgg" ) abgg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abgb" ) abgb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abga" ) abga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abbr" ) abbr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abbg" ) abbg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abbb" ) abbb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abba" ) abba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abar" ) abar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abag" ) abag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abab" ) abab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "abaa" ) abaa{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aarr" ) aarr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aarg" ) aarg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aarb" ) aarb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aara" ) aara{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aagr" ) aagr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aagg" ) aagg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aagb" ) aagb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aaga" ) aaga{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aabr" ) aabr{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aabg" ) aabg{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aabb" ) aabb{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aaba" ) aaba{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aaar" ) aaar{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aaag" ) aaag{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aaab" ) aaab{ };
		static constexpr ORB_SWIZZLE_COMPONENT( "aaaa" ) aaaa{ };
	};
}

ORB_NAMESPACE_END
