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
#include <string_view>
#include <vector>

#include "Orbit/Core/Core.h"

ORB_NAMESPACE_BEGIN

template< typename T >
class Span
{
public:

	Span( void )
		: m_ptr   { nullptr }
		, m_count { 0 }
	{
	}

	template< size_t N >
	Span( const T ( &arr )[ N ] )
		: m_ptr   { arr }
		, m_count { N }
	{
	}

	Span( std::initializer_list< T > args )
		: m_ptr   { args.begin() }
		, m_count { args.size() }
	{
	}

	Span( const std::vector< T >& vec )
		: m_ptr   { vec.data() }
		, m_count { vec.size() }
	{
	}

	Span( const T* data, size_t count )
		: m_ptr   { data }
		, m_count { count }
	{
	}

public:

	const T* begin( void ) const { return m_ptr; }
	const T* end  ( void ) const { return ( m_ptr + m_count ); }

private:

	const T* m_ptr;
	size_t   m_count;

};

using ByteSpan = Span< uint8_t >;

ORB_NAMESPACE_END
