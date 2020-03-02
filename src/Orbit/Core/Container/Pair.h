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

#include <utility>

ORB_NAMESPACE_BEGIN

template< typename KeyType, typename ValueType >
class Pair
{
public:

	constexpr Pair( void )
		: key_  { }
		, value_{ }
	{
	}

	constexpr Pair( const Pair& other )
		: key_  ( other.key_ )
		, value_( other.value_ )
	{
	}

	constexpr Pair( Pair&& other )
		: key_  ( std::move( other.key_ ) )
		, value_( std::move( other.value_ ) )
	{
		other.key_   = { };
		other.value_ = { };
	}

	constexpr Pair( const KeyType& key, const ValueType& value )
		: key_  { key }
		, value_{ value }
	{
	}

public:

	constexpr KeyType&         Key  ( void )       { return key_; }
	constexpr const KeyType&   Key  ( void ) const { return key_; }
	constexpr ValueType&       Value( void )       { return value_; }
	constexpr const ValueType& Value( void ) const { return value_; }

public:

	constexpr Pair& operator=( const Pair& other )
	{
		key_   = other.key_;
		value_ = other.value_;

		return *this;
	}

	constexpr Pair& operator=( Pair&& other )
	{
		key_   = std::move( other.key_ );
		value_ = std::move( other.value_ );

		other.key_   = { };
		other.value_ = { };

		return *this;
	}

public:

	constexpr bool operator==( const Pair& other ) const { return ( key_ == other.key_ ); }
	constexpr bool operator< ( const Pair& other ) const { return ( key_ <  other.key_ ); }
	constexpr bool operator> ( const Pair& other ) const { return ( key_ >  other.key_ ); }

private:

	KeyType   key_;
	ValueType value_;

};

ORB_NAMESPACE_END
