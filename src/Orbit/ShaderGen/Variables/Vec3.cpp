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

#include "Vec3.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	Vec3::Vec3( const Variable& a )
		: Variable( "vec3( " + a.GetValue() + " )", DataType::FVec3 )
	{
		assert( a.GetDataType() == DataType::FVec3 );

		a.SetUsed();
	}

	Vec3::Vec3( const Variable& a, const Variable& b )
		: Variable( "vec3( " + a.GetValue() + ", " + b.GetValue() + " )", DataType::FVec3 )
	{
		assert( ( ( a.GetDataType() == DataType::FVec2 ) && ( b.GetDataType() == DataType::Float ) ) ||
		        ( ( a.GetDataType() == DataType::Float ) && ( b.GetDataType() == DataType::FVec2 ) ) );

		a.SetUsed();
		b.SetUsed();
	}

	Vec3::Vec3( const Variable& a, const Variable& b, const Variable& c )
		: Variable( "vec3( " + a.GetValue() + ", " + b.GetValue() + ", " + c.GetValue() + " )", DataType::FVec3 )
	{
		assert( ( ( a.GetDataType() == DataType::Float ) && ( b.GetDataType() == DataType::Float ) && ( c.GetDataType() == DataType::Float ) ) );

		a.SetUsed();
		b.SetUsed();
		c.SetUsed();
	}
}

ORB_NAMESPACE_END