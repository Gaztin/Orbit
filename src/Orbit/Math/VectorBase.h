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
#include <cmath>
#include <type_traits>

#include "Orbit/Math/Math.h"

ORB_NAMESPACE_BEGIN

template< typename Derived, size_t Size >
class VectorBase
{
public:

	float DotProduct( const VectorBase& rhs ) const
	{
		float d = 0.f;
		for( size_t i = 0; i < Size; ++i )
			d += ( ( *this )[ i ] * rhs[ i ] );
		return d;
	}

	float DotProduct( void ) const
	{
		return DotProduct( *this );
	}

	float Length( void ) const
	{
		return sqrtf( DotProduct() );
	}

	void Normalize( void )
	{
		*this /= Length();
	}

public:

	VectorBase& operator=( const VectorBase& other )
	{
		for( size_t i = 0; i < Size; ++i )
			( *this )[ i ] = other[ i ];
		return *this;
	}

	Derived operator+( const VectorBase& rhs ) const
	{
		Derived v;
		for( size_t i = 0; i < Size; ++i )
			v[ i ] = ( *this )[ i ] + rhs[ i ];
		return v;
	}

	Derived operator-( const VectorBase& rhs ) const
	{
		Derived v;
		for( size_t i = 0; i < Size; ++i )
			v[ i ] = ( *this )[ i ] - rhs[ i ];
		return v;
	}

	Derived operator*( float scalar ) const
	{
		Derived v;
		for( size_t i = 0; i < Size; ++i )
			v[ i ] = ( *this )[ i ] * scalar;
		return v;
	}

	Derived operator/( float scalar ) const
	{
		Derived v;
		for( size_t i = 0; i < Size; ++i )
			v[ i ] = ( *this )[ i ] / scalar;
		return v;
	}

	Derived& operator+=( const VectorBase& rhs )
	{
		for( size_t i = 0; i < Size; ++i )
			( *this )[ i ] += rhs[ i ];

		return Self();
	}

	Derived& operator-=( const VectorBase& rhs )
	{
		for( size_t i = 0; i < Size; ++i )
			( *this )[ i ] -= rhs[ i ];

		return Self();
	}

	Derived& operator*=( float scalar )
	{
		for( size_t i = 0; i < Size; ++i )
			( *this )[ i ] *= scalar;

		return Self();
	}

	Derived& operator/=( float scalar )
	{
		for( size_t i = 0; i < Size; ++i )
			( *this )[ i ] /= scalar;

		return Self();
	}

	float& operator[]( size_t i )
	{
		float* ptr = reinterpret_cast< float* >( this );
		return ptr[ i ];
	}

	const float& operator[]( size_t i ) const
	{
		const float* ptr = reinterpret_cast< const float* >( this );
		return ptr[ i ];
	}

public:

	float*       begin ( void )       { return &( *this )[ 0 ]; }
	const float* begin ( void ) const { return &( *this )[ 0 ]; }
	float*       end   ( void )       { return &( *this )[ Size ]; }
	const float* end   ( void ) const { return &( *this )[ Size ]; }

private:

	const Derived& Self( void ) const { return *reinterpret_cast< const Derived* >( this ); }
	Derived&       Self( void )       { return *reinterpret_cast< Derived*       >( this ); }

};

ORB_NAMESPACE_END
