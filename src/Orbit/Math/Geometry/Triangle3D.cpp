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

#include "Triangle3D.h"

ORB_NAMESPACE_BEGIN

Triangle3D::Triangle3D( Vector3 point_a, Vector3 point_b, Vector3 point_c )
	: point_a_( point_a )
	, point_b_( point_b )
	, point_c_( point_c )
{
}

bool Triangle3D::IsClockwiseAround( Vector3 axis ) const
{
	const Vector3 cross = ( point_b_ - point_a_ ).CrossProduct( point_c_ - point_b_ );

	return std::signbit( cross.DotProduct( axis ) );
}

ORB_NAMESPACE_END
