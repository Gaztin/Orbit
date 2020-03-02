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

#include "Matrix4.h"

#include <cmath>

ORB_NAMESPACE_BEGIN

Matrix4::Matrix4( float diagonal )
{
	for( size_t i = 0; i < 16; ++i )
		( *this )[ i ] = ( i % 5 == 0 ) ? diagonal : 0.0f;
}

Matrix4::Matrix4( std::initializer_list< float > elements )
{
	const size_t size = std::min( static_cast< size_t >( 16u ), elements.size() );

	for( size_t i = 0; i < size; ++i )
		( *this )[ i ] = *( elements.begin() + i );
}

void Matrix4::Translate( const Vector3& translation )
{
	Matrix4&      self = *this;
	const Vector4 t4( translation.x, translation.y, translation.z, 1.f );

	for( size_t i = 0; i < 4; ++i )
		self( i, 3 ) = Vector4( self( i, 0 ), self( i, 1 ), self( i, 2 ), self( i, 3 ) ).DotProduct( t4 );
}

void Matrix4::RotateX( float rotation )
{
	const float c = cosf( rotation );
	const float s = sinf( rotation );
	Matrix4     rot;

	rot( 1, 1 ) =  c;
	rot( 1, 2 ) = -s;
	rot( 2, 1 ) =  s;
	rot( 2, 2 ) =  c;

	*this *= rot;
}

void Matrix4::RotateY( float rotation )
{
	const float c = cosf( rotation );
	const float s = sinf( rotation );
	Matrix4     rot;

	rot( 0, 0 ) =  c;
	rot( 0, 2 ) = -s;
	rot( 2, 0 ) =  s;
	rot( 2, 2 ) =  c;

	*this *= rot;
}

void Matrix4::RotateZ( float rotation )
{
	const float c = cosf( rotation );
	const float s = sinf( rotation );
	Matrix4     rot;

	rot( 0, 0 ) =  c;
	rot( 0, 1 ) = -s;
	rot( 1, 0 ) =  s;
	rot( 1, 1 ) =  c;

	*this *= rot;
}

void Matrix4::Rotate( const Vector3& rotation )
{
	Matrix4 rotx;
	Matrix4 roty;
	Matrix4 rotz;

	rotx.RotateX( rotation.x );
	roty.RotateY( rotation.y );
	rotz.RotateZ( rotation.z );

	*this *= ( rotx * roty * rotz );
}

void Matrix4::Scale( const Vector3& scale )
{
	Matrix4 m;
	m( 0, 0 ) = scale.x;
	m( 1, 1 ) = scale.y;
	m( 2, 2 ) = scale.z;

	*this *= m;
}

void Matrix4::Transpose( void )
{
	Matrix4& self = *this;
	Matrix4  temp( self );

	for( size_t x = 0; x < 4; ++x )
	{
		for( size_t y = 0; y < 4; ++y )
			self( x, y ) = temp( y, x );
	}
}

void Matrix4::Invert( void )
{
	Matrix4 minors;
	for( size_t column = 0; column < 4; ++column )
	{
		for( size_t row = 0; row < 4; ++row )
			minors( column, row ) = GetDeterminant3x3( column, row );
	}

	Matrix4 cofactors;
	for( size_t row = 0; row < 4; ++row )
	{
		for( size_t column = 0; column < 4; ++column )
		{
			float sign = ( ( column & 1 ) != ( row & 1 ) ) ? -1.0f : 1.0f;
			cofactors( column, row ) = minors( column, row ) * sign;
		}
	}

	cofactors.Transpose();

	const float one_over_determinant = 1.0f / ( ( *this )[ 0 ] * minors[ 0 ] -
	                                            ( *this )[ 1 ] * minors[ 1 ] +
	                                            ( *this )[ 2 ] * minors[ 2 ] -
	                                            ( *this )[ 3 ] * minors[ 3 ] );
	for( size_t i = 0; i < 16; ++i )
		( *this )[ i ] = cofactors[ i ] * one_over_determinant;
}

void Matrix4::SetIdentity( void )
{
	for( size_t i = 0; i < 16; ++i )
		( *this )[ i ] = ( i % 5 == 0 ) ? 1.0f : 0.0f;
}

void Matrix4::SetPerspective( float aspect_ratio, float fov, float near_clip, float far_clip )
{
	const float fov_tangent = tanf( fov / 2 );

	( *this )[  0 ] = 1.0f / ( aspect_ratio * fov_tangent );
	( *this )[  1 ] = 0.0f;
	( *this )[  2 ] = 0.0f;
	( *this )[  3 ] = 0.0f;
	( *this )[  4 ] = 0.0f;
	( *this )[  5 ] = 1.0f / fov_tangent;
	( *this )[  6 ] = 0.0f;
	( *this )[  7 ] = 0.0f;
	( *this )[  8 ] = 0.0f;
	( *this )[  9 ] = 0.0f;
	( *this )[ 10 ] = far_clip / ( far_clip - near_clip );
	( *this )[ 11 ] = 1.0f;
	( *this )[ 12 ] = 0.0f;
	( *this )[ 13 ] = 0.0f;
	( *this )[ 14 ] = ( far_clip * near_clip ) / ( near_clip - far_clip );
	( *this )[ 15 ] = 0.0f;
}

float Matrix4::GetDeterminant( void ) const
{
	return ( ( *this )[ 0 ] * GetDeterminant3x3( 0, 0 ) -
	         ( *this )[ 1 ] * GetDeterminant3x3( 1, 0 ) +
	         ( *this )[ 2 ] * GetDeterminant3x3( 2, 0 ) -
	         ( *this )[ 3 ] * GetDeterminant3x3( 3, 0 ) );
}

float Matrix4::GetDeterminant3x3( size_t column, size_t row ) const
{
	const Matrix4& self = *this;
	const size_t   c1   = ( column > 0 ) ? 0 : 1;
	const size_t   c2   = ( column > 1 ) ? 1 : 2;
	const size_t   c3   = ( column > 2 ) ? 2 : 3;
	const size_t   r1   = ( row    > 0 ) ? 0 : 1;
	const size_t   r2   = ( row    > 1 ) ? 1 : 2;
	const size_t   r3   = ( row    > 2 ) ? 2 : 3;

	return self( c1, r1 ) * ( self( c2, r2 ) * self( c3, r3 ) - self( c3, r2 ) * self( c2, r3 ) ) -
	       self( c2, r1 ) * ( self( c1, r2 ) * self( c3, r3 ) - self( c3, r2 ) * self( c1, r3 ) ) +
	       self( c3, r1 ) * ( self( c1, r2 ) * self( c2, r3 ) - self( c2, r2 ) * self( c1, r3 ) );
}

Matrix4 Matrix4::Transposed( void ) const
{
	Matrix4 result( *this );
	result.Transpose();

	return result;
}

Matrix4 Matrix4::Inverted( void ) const
{
	Matrix4 result( *this );
	result.Invert();

	return result;
}

Matrix4 Matrix4::operator*( const Matrix4& rhs ) const
{
	Matrix4 ret;

	for( size_t row = 0; row < 4; ++row )
	{
		for( size_t col = 0; col < 4; ++col )
		{
			ret( col, row ) = ( *this )( 0, row ) * rhs( col, 0 ) +
			                  ( *this )( 1, row ) * rhs( col, 1 ) +
			                  ( *this )( 2, row ) * rhs( col, 2 ) +
			                  ( *this )( 3, row ) * rhs( col, 3 );
		}
	}

	return ret;
}

Vector4 Matrix4::operator*( const Vector4& rhs ) const
{
	Vector4 ret;

	for( size_t row = 0; row < 4; ++row )
	{
		for( size_t col = 0; col < 4; ++col )
			ret[ row ] += ( *this )( col, row ) * rhs[ col ];
	}

	return ret;
}

Matrix4& Matrix4::operator*=( const Matrix4& rhs )
{
	return( *this = ( ( *this ) * rhs ) );
}

Matrix4& Matrix4::operator=( const Matrix4& rhs )
{
	for( size_t i = 0; i < 16; ++i )
		( *this )[ i ] = rhs[ i ];

	return *this;
}

ORB_NAMESPACE_END
