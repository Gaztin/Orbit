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

#include "orbit/core/hash_traits.h"

#include <functional>
#include <string_view>

namespace orb
{
	template< typename ValueType = size_t >
	class hash_view
	{
	public:

		constexpr hash_view()
			: m_value{ }
		{ }

		constexpr hash_view( std::string_view str )
			: m_value( hash( str ) )
		{ }

		constexpr ValueType get_value() { return m_value; }

	private:

		/* FNV-1a hash function as per http://isthe.com/chongo/tech/comp/fnv/ */
		constexpr ValueType hash( std::string_view str )
		{
			using hash_traits = hash_traits_FNV_1a< sizeof( ValueType ) >;
			ValueType val = hash_traits::OffsetBasis;
			for( size_t i = 0; i < str.length(); ++i )
			{
				val ^= static_cast< ValueType >( str[ i ] );
				val *= hash_traits::Prime;
			}
			return val;
		}

	private:

		ValueType m_value;

	};
}
