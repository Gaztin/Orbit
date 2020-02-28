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

#include <string_view>
#include <type_traits>

ORB_NAMESPACE_BEGIN

template< typename T >
constexpr T FromString( std::string_view str )
{
	if constexpr( std::is_integral_v< T > )
	{
		if constexpr( std::is_signed_v< T > )
		{
			/**/ if constexpr( sizeof( T ) <= sizeof( long      ) ) return std::strtol(  str.data(), nullptr, 10 );
			else if constexpr( sizeof( T ) <= sizeof( long long ) ) return std::strtoll( str.data(), nullptr, 10 );
		}
		else
		{
			/**/ if constexpr( sizeof( T ) <= sizeof( unsigned long      ) ) return std::strtoul(  str.data(), nullptr, 10 );
			else if constexpr( sizeof( T ) <= sizeof( unsigned long long ) ) return std::strtoull( str.data(), nullptr, 10 );
		}
	}
	else if constexpr( std::is_floating_point_v< T > )
	{
		/**/ if constexpr( std::is_same_v< T, float       > ) return std::strtof(  str.data(), nullptr );
		else if constexpr( std::is_same_v< T, double      > ) return std::strtod(  str.data(), nullptr );
		else if constexpr( std::is_same_v< T, long double > ) return std::strtold( str.data(), nullptr );
	}
}

ORB_NAMESPACE_END
