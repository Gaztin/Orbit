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

#include "Plane.h"

ORB_NAMESPACE_BEGIN

Plane::Plane( void )
	: normal      ( 0.0f, 1.0f, 0.0f )
	, displacement( 0.0f )
{
}

Plane::Plane( const Vector3& normal, float displacement )
	: normal      ( normal )
	, displacement( displacement )
{
}

Plane::PlaneIntersectionResult Plane::Intersect( const Plane& other ) const
{
	const Vector3 orthogonal = normal.CrossProduct( other.normal );

	// Are planes parallel?
	if( orthogonal.x == 0.0f && orthogonal.y == 0.0f && orthogonal.z == 0.0f )
	{
		// Planes are the same, entire plane intersects
		if( displacement == other.displacement )
			return *this;

		// Planes are parallel, which means no intersection
		return { };
	}

	return Line( orthogonal, Vector2( displacement, other.displacement ) );
}

ORB_NAMESPACE_END
