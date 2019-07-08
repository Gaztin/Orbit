/*
 * Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include <cstdint>

/* See https://stackoverflow.com/a/22253389 */
#ifdef major
#  undef major
#endif
#ifdef minor
#  undef minor
#endif

namespace orb
{
	class version
	{
	public:
		constexpr version()                                               : major( 0 ), minor( 0 ), patch( 0 ) { }
		constexpr explicit version( uint8_t major )                       : major( major ), minor( 0 ), patch( 0 ) { }
		constexpr version( uint8_t major, uint8_t minor )                 : major( major ), minor( minor ), patch( 0 ) { }
		constexpr version( uint8_t major, uint8_t minor, uint16_t patch ) : major( major ), minor( minor ), patch( patch ) { }

		constexpr bool operator== ( const version& v ) const { return ( major == v.major && minor == v.minor && patch == v.patch ); }
		constexpr bool operator<  ( const version& v ) const { return ( major < v.major || ( major == v.major && ( minor < v.minor || ( minor == v.minor && ( patch < v.patch ) ) ) ) ); }
		constexpr bool operator>  ( const version& v ) const { return ( major > v.major || ( major == v.major && ( minor > v.minor || ( minor == v.minor && ( patch > v.patch ) ) ) ) ); }
		constexpr bool operator<= ( const version& v ) const { return ( *this == v || *this < v ); }
		constexpr bool operator>= ( const version& v ) const { return ( *this == v || *this > v ); }

		constexpr operator uint32_t() const
		{
			return ( static_cast< uint32_t >( major ) << 24 ) |
			       ( static_cast< uint32_t >( minor ) << 16 ) |
			       ( static_cast< uint32_t >( patch ) );
		}

		uint8_t  major;
		uint8_t  minor;
		uint16_t patch;
	};
}
