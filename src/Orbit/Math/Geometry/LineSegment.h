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
#include "Orbit/Math/Vector/Vector3.h"

ORB_NAMESPACE_BEGIN

class ORB_API_MATH LineSegment
{
public:

	LineSegment( void ) = default;
	LineSegment( const Vector3& start, const Vector3& end );

public:

	Vector3 PointAt      ( float fraction ) const;
	Vector3 Center       ( void )           const;
	Vector3 Direction    ( void )           const;
	float   Length       ( void )           const;
	float   LengthSquared( void )           const;

public:

	Vector3 start;
	Vector3 end;

};

ORB_NAMESPACE_END
