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

namespace orb
{
	class version
	{
	public:
		constexpr version()                                               : m_major( 0 ), m_minor( 0 ), m_patch( 0 ) { }
		constexpr explicit version( uint8_t major )                       : m_major( major ), m_minor( 0 ), m_patch( 0 ) { }
		constexpr version( uint8_t major, uint8_t minor )                 : m_major( major ), m_minor( minor ), m_patch( 0 ) { }
		constexpr version( uint8_t major, uint8_t minor, uint16_t patch ) : m_major( major ), m_minor( minor ), m_patch( patch ) { }

		uint8_t  get_major() const { return m_major; }
		uint8_t  get_minor() const { return m_minor; }
		uint16_t get_patch() const { return m_patch; }

		constexpr bool operator== ( const version& v ) const { return ( m_major == v.m_major && m_minor == v.m_minor && m_patch == v.m_patch ); }
		constexpr bool operator<  ( const version& v ) const { return ( m_major < v.m_major || ( m_major == v.m_major && ( m_minor < v.m_minor || ( m_minor == v.m_minor && ( m_patch < v.m_patch ) ) ) ) ); }
		constexpr bool operator>  ( const version& v ) const { return ( m_major > v.m_major || ( m_major == v.m_major && ( m_minor > v.m_minor || ( m_minor == v.m_minor && ( m_patch > v.m_patch ) ) ) ) ); }
		constexpr bool operator<= ( const version& v ) const { return ( *this == v || *this < v ); }
		constexpr bool operator>= ( const version& v ) const { return ( *this == v || *this > v ); }

	private:
		uint8_t  m_major;
		uint8_t  m_minor;
		uint16_t m_patch;
	};
}
