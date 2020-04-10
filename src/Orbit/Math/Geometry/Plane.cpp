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

Vector3 Plane::Center( void ) const
{
	return ( normal * displacement );
}

Plane::PlaneIntersectionResult Plane::Intersect( const Plane& other ) const
{
	const Vector3 orthogonal = normal.CrossProduct( other.normal );

	// Are planes parallel?
	if( orthogonal.x == 0.0f && orthogonal.y == 0.0f && orthogonal.z == 0.0f )
	{
		// Check if planes are the same
		if( displacement == other.displacement )
			return *this;

		// Planes are parallel, which means no intersection
		return { };
	}

	return Line( orthogonal, Vector2( displacement, other.displacement ) );
}

Plane::LineIntersectionResult Plane::Intersect( const Line& line ) const
{
	// Check if plane contains line
	if( normal.DotProduct( line.direction ) == 0.0f )
		return line;

//////////////////////////////////////////////////////////////////////////

	const Vector3 point_on_plane  = ( normal * displacement );
	const Vector3 line_start      = ( line.direction * -100.0f ); // #TODO: Fix magic number
	const Vector3 plane_to_line   = ( line_start - point_on_plane );
	const float   travel_distance = ( ( -normal ).DotProduct( plane_to_line ) / normal.DotProduct( line.direction ) );

	return ( line_start + ( line.direction * travel_distance ) );
}

Plane::LineSegmentIntersectionResult Plane::Intersect( const LineSegment& line_segment ) const
{
	const float start_dist = normal.DotProduct( line_segment.start );
	const float end_dist   = normal.DotProduct( line_segment.end );

	// Is the line segment a point on the plane?
	if( ( start_dist == end_dist ) && ( start_dist == displacement ) )
		return { line_segment.start };

	// Start must be in front of plane, and end behind it
	if( ( start_dist > displacement ) && ( end_dist < displacement ) )
	{
		const float fraction = ( start_dist - displacement ) / ( start_dist - end_dist );

		return line_segment.PointAt( fraction );
	}

	return { };
}

ORB_NAMESPACE_END
