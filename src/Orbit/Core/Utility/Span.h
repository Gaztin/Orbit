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
#include "Orbit/Core/IO/Asset.h"

#include <memory>
#include <string_view>
#include <vector>

ORB_NAMESPACE_BEGIN

template< typename T >
class Span
{
public:

	Span( void )
		: ptr_  { nullptr }
		, count_{ 0 }
	{
	}

	template< size_t N >
	Span( const T ( &arr )[ N ] )
		: ptr_  { arr }
		, count_{ N }
	{
	}

	Span( std::initializer_list< T > args )
		: ptr_  { args.begin() }
		, count_{ args.size() }
	{
	}

	Span( const std::vector< T >& vec )
		: ptr_  { vec.data() }
		, count_{ vec.size() }
	{
	}

	Span( const Asset& asset )
		: ptr_  { asset.GetData() }
		, count_{ asset.GetSize() }
	{
	}

	Span( const T* data, size_t count )
		: ptr_  { data }
		, count_{ count }
	{
	}

public:

	const T* Ptr ( void ) const { return ptr_; }
	size_t   Size( void ) const { return count_; }

	std::unique_ptr< T[] > Copy( void ) const
	{
		std::unique_ptr< T[] > ptr( new T[ count_ ] );
		std::copy( begin(), end(), &ptr[ 0 ] );

		return ptr;
	}

public:

	const T* begin( void ) const { return ptr_; }
	const T* end  ( void ) const { return ( ptr_ + count_ ); }

public:

	operator bool( void ) const { return ( ptr_ != nullptr ); }

private:

	const T* ptr_;
	size_t   count_;

};

using ByteSpan = Span< uint8_t >;

ORB_NAMESPACE_END
