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

#include "Vec4.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	Vec4::Vec4( const Variable& a )
		: Variable( "vec4( " + a.GetValue() + " )", DataType::FVec4 )
	{
		assert( a.GetDataType() == DataType::FVec4 );
	}

	Vec4::Vec4( const Variable& a, const Variable& b )
		: Variable( "vec4( " + a.GetValue() + ", " + b.GetValue() + " )", DataType::FVec4 )
	{
		assert( ( ( a.GetDataType() == DataType::FVec3 ) && ( b.GetDataType() == DataType::Float ) ) ||
		        ( ( a.GetDataType() == DataType::FVec2 ) && ( b.GetDataType() == DataType::FVec2 ) ) ||
		        ( ( a.GetDataType() == DataType::Float ) && ( b.GetDataType() == DataType::FVec3 ) ) );
	}

	Vec4::Vec4( const Variable& a, const Variable& b, const Variable& c )
		: Variable( "vec4( " + a.GetValue() + ", " + b.GetValue() + ", " + c.GetValue() + " )", DataType::FVec4 )
	{
		assert( ( ( a.GetDataType() == DataType::FVec2 ) && ( b.GetDataType() == DataType::Float ) && ( c.GetDataType() == DataType::Float ) ) ||
		        ( ( a.GetDataType() == DataType::Float ) && ( b.GetDataType() == DataType::FVec2 ) && ( c.GetDataType() == DataType::Float ) ) ||
		        ( ( a.GetDataType() == DataType::Float ) && ( b.GetDataType() == DataType::Float ) && ( c.GetDataType() == DataType::FVec2 ) ) );
	}

	Vec4::Vec4( const Variable& a, const Variable& b, const Variable& c, const Variable& d )
		: Variable( "vec4( " + a.GetValue() + ", " + b.GetValue() + ", " + c.GetValue() + ", " + d.GetValue() + " )", DataType::FVec4 )
	{
		assert( ( ( a.GetDataType() == DataType::Float ) && ( b.GetDataType() == DataType::Float ) && ( c.GetDataType() == DataType::Float ) && ( d.GetDataType() == DataType::Float ) ) );
	}
}

ORB_NAMESPACE_END
