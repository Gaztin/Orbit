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
#include "Orbit/Core/Core.h"

#include <cstdlib>

ORB_NAMESPACE_BEGIN

class Color
{
public:

	constexpr Color( void )
		: r( 0.0f )
		, g( 0.0f )
		, b( 0.0f )
		, a( 1.0f )
	{
	}

	constexpr Color( float r, float g, float b, float a = 1.0f )
		: r( r )
		, g( g )
		, b( b )
		, a( a )
	{
	}

public:

	constexpr float&       operator[]( size_t i )       { return ( &r )[ i ]; }
	constexpr const float& operator[]( size_t i ) const { return ( &r )[ i ]; }

public:

	static Color Random( void )
	{
		Color color;
		color.r = rand() / static_cast< float >( RAND_MAX );
		color.g = rand() / static_cast< float >( RAND_MAX );
		color.b = rand() / static_cast< float >( RAND_MAX );

		return color;
	}

public:

	float r;
	float g;
	float b;
	float a;

};

ORB_NAMESPACE_END
