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

#include <type_traits>

ORB_NAMESPACE_BEGIN

#define ORB_ENABLE_BITMASKING( ENUM )        \
    template<>                               \
    struct EnableBitMasking< ENUM >          \
    {                                        \
        static constexpr bool enable = true; \
    }

template< typename E >
struct EnableBitMasking
{
	static constexpr bool enable = false;
};

template< typename E >
constexpr bool enable_bit_masking_v = EnableBitMasking< E >::enable;

template< typename E >
typename std::enable_if_t< enable_bit_masking_v< E >, bool > operator!( E rhs )
{
	return !static_cast< std::underlying_type_t< E > >( rhs );
}

template< typename E >
typename std::enable_if_t< enable_bit_masking_v< E >, E > operator~( E rhs )
{
	return static_cast< E >( ~static_cast< std::underlying_type_t< E > >( rhs ) );
}

template< typename E >
typename std::enable_if_t< enable_bit_masking_v< E >, E > operator|( E lhs, E rhs )
{
	return static_cast< E >( static_cast< std::underlying_type_t< E > >( lhs ) | static_cast< std::underlying_type_t< E > >( rhs ) );
}

template< typename E >
typename std::enable_if_t< enable_bit_masking_v< E >, E > operator&( E lhs, E rhs )
{
	return static_cast< E >( static_cast< std::underlying_type_t< E > >( lhs ) & static_cast< std::underlying_type_t< E > >( rhs ) );
}

template< typename E >
typename std::enable_if_t< enable_bit_masking_v< E >, E > operator^( E lhs, E rhs )
{
	return static_cast< E >( static_cast< std::underlying_type_t< E > >( lhs ) ^ static_cast< std::underlying_type_t< E > >( rhs ) );
}

template< typename E >
typename std::enable_if_t< enable_bit_masking_v< E >, E& > operator|=( E& lhs, E rhs )
{
	return ( lhs = static_cast< E >( static_cast< std::underlying_type_t< E > >( lhs ) | static_cast< std::underlying_type_t< E > >( rhs ) ) );
}

template< typename E >
typename std::enable_if_t< enable_bit_masking_v< E >, E& > operator&=( E& lhs, E rhs )
{
	return ( lhs = static_cast< E >( static_cast< std::underlying_type_t< E > >( lhs ) & static_cast< std::underlying_type_t< E > >( rhs ) ) );
}

template< typename E >
typename std::enable_if_t< enable_bit_masking_v< E >, E& > operator^=( E& lhs, E rhs )
{
	return ( lhs = static_cast< E >( static_cast< std::underlying_type_t< E > >( lhs ) ^ static_cast< std::underlying_type_t< E > >( rhs ) ) );
}

ORB_NAMESPACE_END
