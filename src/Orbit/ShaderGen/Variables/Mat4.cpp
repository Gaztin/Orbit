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

#include "Mat4.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	Mat4::Mat4( const Variable& value )
		: Variable( "mat4( " + value.GetValue() + " )", DataType::Mat4 )
	{
		assert( value.GetDataType() == DataType::Mat4 );

		value.SetUsed();
	}
}

ORB_NAMESPACE_END
