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

#include <string>
#include <type_traits>
#include <utility>
#include <variant>

ORB_NAMESPACE_BEGIN

template< typename... Types > struct Overload : Types... { using Types::operator()...; };
template< typename... Types > Overload( Types... ) -> Overload< Types... >;

template< typename T, size_t C >
constexpr size_t CountOf( T( & )[ C ] )
{
	return C;
}

/* Integer sequences by courtesy of https://stackoverflow.com/a/16387374 */

template< size_t... Is >
struct Sequence { };

template< size_t N, size_t... Is >
struct MakeSequence : MakeSequence< ( N - 1 ), ( N - 1 ), Is... > { };

template< size_t... Is >
struct MakeSequence< 0, Is... > : Sequence< Is... > { };

/* Get type index within variant. Courtesy of https://stackoverflow.com/a/52303687 */

template< typename >
struct Tag { };

template< typename T, typename... Ts >
constexpr size_t variant_index_v = std::variant< Tag< Ts >... >( Tag< T >{ } ).index();

template< typename T, typename Variant >
struct UniqueIndex;

template< typename T, typename... Ts >
struct UniqueIndex< T, std::variant< Ts... > > : std::integral_constant< size_t, variant_index_v< T, Ts... > >
{
};

template< typename T, typename... Ts >
constexpr auto unique_index_v = UniqueIndex< T, Ts... >::value;

/* Function argument type deduction by courtesy of https://stackoverflow.com/a/35348334 */

template< typename Ret, typename Arg, typename... Rest >
constexpr Arg first_argument_helper( Ret( * )( Arg, Rest... ) );

template< typename Ret, typename F, typename Arg, typename... Rest >
constexpr Arg first_argument_helper( Ret( F::* )( Arg, Rest... ) );

template< typename Ret, typename F, typename Arg, typename... Rest >
constexpr Arg first_argument_helper( Ret( F::* )( Arg, Rest... ) const );

template< typename F >
constexpr decltype( first_argument_helper( &F::operator() ) ) first_argument_helper( F );

template< typename T >
using FirstArgument = decltype( first_argument_helper( std::declval< T >() ) );

/* FNV constants (reference: https://tools.ietf.org/html/draft-eastlake-fnv-17) */

template< uint32_t HashSize >
struct HashTraitsFNV;

template<>
struct HashTraitsFNV< 4 >
{
	using ValueType = uint32_t;

	static constexpr ValueType prime        =   16777619u;
	static constexpr ValueType offset_basis = 2166136261u;
};

template<>
struct HashTraitsFNV< 8 >
{
	using ValueType = uint64_t;

	static constexpr ValueType prime        =        1099511628211ull;
	static constexpr ValueType offset_basis = 14695981039346656037ull;
};

template< typename... Args >
std::string Format( const char* fmt, Args... args )
{
	const int len = snprintf( nullptr, 0, fmt, args... );
	if( len < 0 )
		return std::string( fmt );

	std::string res( len, '\0' );
	snprintf( &res[ 0 ], len + 1, fmt, args... );
	return res;
}

ORB_NAMESPACE_END
