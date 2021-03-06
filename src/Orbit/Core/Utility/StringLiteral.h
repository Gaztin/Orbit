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
#include "Orbit/Core/Utility/Utility.h"

#include <cstddef>

ORB_NAMESPACE_BEGIN

template< char... Chars >
class StringLiteral
{
public:

	static constexpr char value[ sizeof...( Chars ) + 1 ] = { Chars..., '\0' };

public:

	static constexpr size_t Length( void )
	{
		return Length< 0, Chars... >();
	}

	static constexpr bool HasDuplicateChar( void )
	{
		return HasDuplicateChar< 0, Chars... >();
	}

private:

	template< size_t Index, char Arg0, char... Args >
	static constexpr size_t Length( void )
	{
		if constexpr( Arg0 == '\0' )
		{
			return Index;
		}
		else
		{
			return Length< Index + 1, Args... >();
		}
	}

	template< size_t Index >
	static constexpr size_t Length( void )
	{
		return Index;
	}

	template< size_t Index, char Arg0, char... Args >
	static constexpr bool HasDuplicateChar( void )
	{
		if constexpr( Arg0 == '\0' )
		{
			return false;
		}
		else
		{
			return ( HasChar< Index, Arg0, Args... >() || HasDuplicateChar< Index + 1, Args... >() );
		}
	}

	template< size_t Index >
	static constexpr bool HasDuplicateChar( void )
	{
		return false;
	}

	template< size_t Index, char Which, char Arg0, char... Args >
	static constexpr bool HasChar( void )
	{
		if constexpr( Which == Arg0 )
		{
			return true;
		}
		else
		{
			return HasChar< Index + 1, Which, Args... >();
		}
	}

	template< size_t Index, char Which >
	static constexpr bool HasChar( void )
	{
		return false;
	}

};

template< size_t Index, size_t Length >
constexpr char StringLiteralGrab( const char ( &str )[ Length ] )
{
	Use( str );

	if constexpr( Index < Length ) return str[ Index ];
	else                           return '\0';
}

#define ORB_STRING_LITERAL_32( STRING )                  \
    decltype( ORB_NAMESPACE StringLiteral<               \
        ORB_NAMESPACE StringLiteralGrab<  0 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  1 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  2 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  3 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  4 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  5 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  6 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  7 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  8 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab<  9 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 10 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 11 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 12 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 13 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 14 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 15 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 16 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 17 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 18 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 19 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 20 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 21 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 22 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 23 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 24 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 25 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 26 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 27 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 28 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 29 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 30 >( STRING ), \
        ORB_NAMESPACE StringLiteralGrab< 31 >( STRING ) >() )

ORB_NAMESPACE_END
