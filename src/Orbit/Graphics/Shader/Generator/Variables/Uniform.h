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
#include "Orbit/Graphics/Shader/Generator/Variables/IVariable.h"

ORB_NAMESPACE_BEGIN

namespace ShaderGen { namespace Variables
{
	class Float;
	class Vec2;
	class Vec3;
	class Vec4;
	class Mat4;

	template< DataType DT >
	struct DataTypeTraits
	{
		static constexpr DataType data_type = DT;
	};

	template< typename T >
	struct UniformTraits
	{
	};

	template<> struct UniformTraits< Float > : DataTypeTraits< DataType::Float > { };
	template<> struct UniformTraits< Vec2  > : DataTypeTraits< DataType::FVec2 > { };
	template<> struct UniformTraits< Vec3  > : DataTypeTraits< DataType::FVec3 > { };
	template<> struct UniformTraits< Vec4  > : DataTypeTraits< DataType::FVec4 > { };
	template<> struct UniformTraits< Mat4  > : DataTypeTraits< DataType::Mat4  > { };

	class ORB_API_GRAPHICS UniformBase : public IVariable
	{
	public:

		UniformBase( DataType type );

	public:

		virtual bool IsArray( void ) const { return false; }

	};

	template< typename T >
	class Uniform : public UniformBase
	{
	public:

		Uniform( void )
			: UniformBase( UniformTraits< T >::data_type )
		{
		}

	};

	class ORB_API_GRAPHICS UniformArrayBase : public UniformBase
	{
	public:

		explicit UniformArrayBase( DataType element_type );

	public:

		bool IsArray( void ) const override { return true; }

	public:

		virtual DataType GetElementType( void ) const = 0;
		virtual size_t   GetArraySize  ( void ) const = 0;

	public:

		IVariable operator[]( const IVariable& index ) const override;

	private:

		DataType m_element_type;

	};

	template< typename T, size_t N >
	class UniformArray : public UniformArrayBase
	{
	public:

		UniformArray( void )
			: UniformArrayBase( UniformTraits< T >::data_type )
		{
		}

	public:

		DataType GetElementType( void ) const override { return UniformTraits< T >::data_type; }
		size_t   GetArraySize  ( void ) const override { return N; }

	};
} }

ORB_NAMESPACE_END
