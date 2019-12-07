/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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
#include "Orbit/Math/Math.h"

ORB_NAMESPACE_BEGIN

namespace MathLiterals
{
	constexpr float operator ""_pi( long double d )        { return ( static_cast< float >( d ) * Pi ); }
	constexpr float operator ""_pi( unsigned long long i ) { return ( static_cast< float >( i ) * Pi ); }
}

namespace UnitLiterals
{
	namespace Metric
	{
		constexpr float operator ""_mm( long double d        ) { return ( static_cast< float >( d * 0.001 ) ); }
		constexpr float operator ""_mm( unsigned long long i ) { return ( static_cast< float >( i * 0.001 ) ); }
		constexpr float operator ""_cm( long double d        ) { return ( static_cast< float >( d * 0.01 ) ); }
		constexpr float operator ""_cm( unsigned long long i ) { return ( static_cast< float >( i * 0.01 ) ); }
		constexpr float operator ""_dm( long double d        ) { return ( static_cast< float >( d * 0.1 ) ); }
		constexpr float operator ""_dm( unsigned long long i ) { return ( static_cast< float >( i * 0.1 ) ); }
		constexpr float operator ""_m ( long double d        ) { return ( static_cast< float >( d ) ); }
		constexpr float operator ""_m ( unsigned long long i ) { return ( static_cast< float >( i ) ); }
		constexpr float operator ""_km( long double d        ) { return ( static_cast< float >( d * 1000.0 ) ); }
		constexpr float operator ""_km( unsigned long long i ) { return ( static_cast< float >( i * 1000.0 ) ); }
	}

	namespace Imperial
	{
		constexpr float operator ""_in ( long double d )        { return ( static_cast< float >( d * 0.0254 ) ); }
		constexpr float operator ""_in ( unsigned long long i ) { return ( static_cast< float >( i * 0.0254 ) ); }
		constexpr float operator ""_ft ( long double d )        { return ( static_cast< float >( d * 0.3048 ) ); }
		constexpr float operator ""_ft ( unsigned long long i ) { return ( static_cast< float >( i * 0.3048 ) ); }
		constexpr float operator ""_yd ( long double d )        { return ( static_cast< float >( d * 0.9144 ) ); }
		constexpr float operator ""_yd ( unsigned long long i ) { return ( static_cast< float >( i * 0.9144 ) ); }
		constexpr float operator ""_ch ( long double d )        { return ( static_cast< float >( d * 20.1168 ) ); }
		constexpr float operator ""_ch ( unsigned long long i ) { return ( static_cast< float >( i * 20.1168 ) ); }
		constexpr float operator ""_fur( long double d )        { return ( static_cast< float >( d * 201.168 ) ); }
		constexpr float operator ""_fur( unsigned long long i ) { return ( static_cast< float >( i * 201.168 ) ); }
		constexpr float operator ""_mi ( long double d )        { return ( static_cast< float >( d * 1609.344 ) ); }
		constexpr float operator ""_mi ( unsigned long long i ) { return ( static_cast< float >( i * 1609.344 ) ); }
	}
}

ORB_NAMESPACE_END

#if defined( ORB_CC_MSVC )
#  pragma warning( pop )
#elif defined( ORB_CC_CLANG )
#  pragma clang diagnostic pop
#endif
