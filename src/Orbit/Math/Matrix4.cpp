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

#include "Matrix4.h"

#include <cmath>

#include "Orbit/Math/Vector3.h"
#include "Orbit/Math/Vector4.h"

ORB_NAMESPACE_BEGIN

Matrix4::Matrix4( float diagonal )
{
	for( size_t i = 0; i < 16; ++i )
	{
		m_elements[ i ] = ( i % 5 == 0 ) ? diagonal : 0.0f;
	}
}

Matrix4::Matrix4( std::initializer_list< float > elements )
{
	const size_t size = std::min( static_cast< size_t >( 16u ), elements.size() );

	for( size_t i = 0; i < size; ++i )
	{
		m_elements[ i ] = *( elements.begin() + i );
	}
}

void Matrix4::Translate( const Vector3& translation )
{
	const Vector4 t4( translation[ 0 ], translation[ 1 ], translation[ 2 ], 1.f );

	for( size_t i = 0; i < 4; ++i )
	{
		m_elements[ 3 * 4 + i ] = Vector4( m_elements[ 0 * 4 + i ], m_elements[ 1 * 4 + i ], m_elements[ 2 * 4 + i ], m_elements[ 3 * 4 + i ] ).DotProduct( t4 );
	}
}

void Matrix4::Rotate( const Vector3& rotation )
{
	const float xcos = cosf( rotation.x );
	const float xsin = sinf( rotation.x );
	const float ycos = cosf( rotation.y );
	const float ysin = sinf( rotation.y );
	const float zcos = cosf( rotation.z );
	const float zsin = sinf( rotation.z );
	Matrix4     rotx;
	Matrix4     roty;
	Matrix4     rotz;

	rotx[ 1 * 4 + 1 ] = xcos;
	rotx[ 2 * 4 + 1 ] = -xsin;
	rotx[ 1 * 4 + 2 ] = xsin;
	rotx[ 2 * 4 + 2 ] = xcos;

	roty[ 0 * 4 + 0 ] = ycos;
	roty[ 2 * 4 + 0 ] = -ysin;
	roty[ 0 * 4 + 2 ] = ysin;
	roty[ 2 * 4 + 2 ] = ycos;

	rotz[ 0 * 4 + 0 ] = zcos;
	rotz[ 1 * 4 + 0 ] = -zsin;
	rotz[ 0 * 4 + 1 ] = zsin;
	rotz[ 1 * 4 + 1 ] = zcos;

	*this *= ( rotx * roty * rotz );
}

void Matrix4::Transpose( void )
{
	Matrix4 temp( *this );
	for( size_t x = 0; x < 4; ++x )
	{
		for( size_t y = 0; y < 4; ++y )
		{
			m_elements[ y * 4 + x ] = temp.m_elements[ x * 4 + y ];
		}
	}
}

void Matrix4::Invert( void )
{
	Matrix4 minors;
	for( size_t column = 0; column < 4; ++column )
	{
		for( size_t row = 0; row < 4; ++row )
		{
			minors( column, row ) = GetDeterminant3x3( column, row );
		}
	}

	Matrix4 cofactors;
	for( size_t row = 0; row < 4; ++row )
	{
		for( size_t column = 0; column < 4; ++column )
		{
			float sign = ( ( column & 1 ) != ( row & 1 ) ) ? -1.0f : 1.0f;
			cofactors[ 4 * row + column ] = minors[ 4 * row + column ] * sign;
		}
	}

	cofactors.Transpose();

	const float one_over_determinant = 1.0f / ( m_elements[ 0 ] * minors[ 0 ] -
	                                            m_elements[ 1 ] * minors[ 1 ] +
	                                            m_elements[ 2 ] * minors[ 2 ] -
	                                            m_elements[ 3 ] * minors[ 3 ] );
	for( size_t i = 0; i < 16; ++i )
	{
		m_elements[ i ] = cofactors[ i ] * one_over_determinant;
	}
}

void Matrix4::SetIdentity( void )
{
	for( size_t i = 0; i < 16; ++i )
	{
		m_elements[ i ] = ( i % 5 == 0 ) ? 1.0f : 0.0f;
	}
}

void Matrix4::SetPerspective( float aspect_ratio, float fov, float near_clip, float far_clip )
{
	const float fov_tangent = tanf( fov / 2 );

	m_elements[ 0  ] = 1.0f / ( aspect_ratio * fov_tangent );
	m_elements[ 1  ] = 0.0f;
	m_elements[ 2  ] = 0.0f;
	m_elements[ 3  ] = 0.0f;
	m_elements[ 4  ] = 0.0f;
	m_elements[ 5  ] = 1.0f / fov_tangent;
	m_elements[ 6  ] = 0.0f;
	m_elements[ 7  ] = 0.0f;
	m_elements[ 8  ] = 0.0f;
	m_elements[ 9  ] = 0.0f;
	m_elements[ 10 ] = far_clip / ( far_clip - near_clip );
	m_elements[ 11 ] = 1.0f;
	m_elements[ 12 ] = 0.0f;
	m_elements[ 13 ] = 0.0f;
	m_elements[ 14 ] = ( far_clip * near_clip ) / ( near_clip - far_clip );
	m_elements[ 15 ] = 0.0f;
}

float Matrix4::GetDeterminant( void ) const
{
	return ( m_elements[ 0 ] * GetDeterminant3x3( 0, 0 ) -
	         m_elements[ 1 ] * GetDeterminant3x3( 1, 0 ) +
	         m_elements[ 2 ] * GetDeterminant3x3( 2, 0 ) -
	         m_elements[ 3 ] * GetDeterminant3x3( 3, 0 ) );
}

float Matrix4::GetDeterminant3x3( size_t column, size_t row ) const
{
	const size_t c1 = ( column > 0 ) ? 0 : 1;
	const size_t c2 = ( column > 1 ) ? 1 : 2;
	const size_t c3 = ( column > 2 ) ? 2 : 3;
	const size_t r1 = ( row    > 0 ) ? 0 : 1;
	const size_t r2 = ( row    > 1 ) ? 1 : 2;
	const size_t r3 = ( row    > 2 ) ? 2 : 3;

	return m_elements[ 4 * r1 + c1 ] * ( m_elements[ 4 * r2 + c2 ] * m_elements[ 4 * r3 + c3 ] - m_elements[ 4 * r2 + c3 ] * m_elements[ 4 * r3 + c2 ] ) -
	       m_elements[ 4 * r1 + c2 ] * ( m_elements[ 4 * r2 + c1 ] * m_elements[ 4 * r3 + c3 ] - m_elements[ 4 * r2 + c3 ] * m_elements[ 4 * r3 + c1 ] ) +
	       m_elements[ 4 * r1 + c3 ] * ( m_elements[ 4 * r2 + c1 ] * m_elements[ 4 * r3 + c2 ] - m_elements[ 4 * r2 + c2 ] * m_elements[ 4 * r3 + c1 ] );
}

Matrix4 Matrix4::operator*( const Matrix4& rhs ) const
{
	return ( Matrix4( *this ) *= rhs );
}

Matrix4& Matrix4::operator*=( const Matrix4& rhs )
{
	std::array< Vector4, 4 > columns;
	std::array< Vector4, 4 > rows;

	for( size_t i = 0; i < 4; ++i )
	{
		columns[ i ] = Vector4( m_elements[ i * 4 + 0 ], m_elements[ i * 4 + 1 ], m_elements[ i * 4 + 2 ], m_elements[ i * 4 + 3 ] );
	}

	for( size_t i = 0; i < 4; ++i )
	{
		rows[ i ] = Vector4( rhs.m_elements[ 0 * 4 + i ], rhs.m_elements[ 1 * 4 + i ], rhs.m_elements[ 2 * 4 + i ], rhs.m_elements[ 3 * 4 + i ] );
	}

	for( size_t row = 0; row < 4; ++row )
	{
		for( size_t col = 0; col < 4; ++col )
		{
			m_elements[ row * 4 + col ] = columns[ row ].DotProduct( rows[ col ] );
		}
	}

	return *this;
}

ORB_NAMESPACE_END
