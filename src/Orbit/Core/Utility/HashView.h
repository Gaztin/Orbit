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

#include <string_view>

ORB_NAMESPACE_BEGIN

template< typename ValueType = size_t >
class HashView
{
public:

	constexpr HashView( void )
		: value_{ }
	{
	}

	constexpr HashView( std::string_view str )
		: value_{ Hash( str ) }
	{
	}

	constexpr ValueType GetValue( void ) const { return value_; }

private:

	/* FNV-1a hash function as per http://isthe.com/chongo/tech/comp/fnv/ */
	constexpr ValueType Hash( std::string_view str ) const
	{
		using HashTraits = HashTraitsFNV< sizeof( ValueType ) >;

		ValueType val = HashTraits::offset_basis;

		for( size_t i = 0; i < str.length(); ++i )
		{
			val ^= static_cast< ValueType >( str[ i ] );
			val *= HashTraits::prime;
		}

		return val;
	}

private:

	ValueType value_;

};

ORB_NAMESPACE_END
