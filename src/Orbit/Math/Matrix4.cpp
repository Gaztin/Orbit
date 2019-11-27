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

void Matrix4::Translate( const Vector3& t )
{
	const Vector4 t4( t[ 0 ], t[ 1 ], t[ 2 ], 1.f );
	for( size_t i = 0; i < 4; ++i )
		m_elements[ 3 * 4 + i ] = Vector4( m_elements[ 0 * 4 + i ], m_elements[ 1 * 4 + i ], m_elements[ 2 * 4 + i ], m_elements[ 3 * 4 + i ] ).DotProduct( t4 );
}

void Matrix4::Rotate( const Vector3& r )
{
	const float sinx = sinf( r.x );
	const float cosx = cosf( r.x );
	const float siny = sinf( r.y );
	const float cosy = cosf( r.y );
	const float sinz = sinf( r.z );
	const float cosz = cosf( r.z );

	*this *= {
		( cosy * cosz ), sinz, siny, 0.f,
		sinz, ( cosx* cosz ), sinx, 0.f,
		siny, ( 0.f - sinx ), ( cosx * cosy ), 0.f,
		0.f, 0.f, 0.f, 1.f,
	};
}

void Matrix4::Transpose()
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

void Matrix4::SetIdentity()
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

Matrix4 Matrix4::operator*( const Matrix4& rhs ) const
{
	return ( Matrix4( *this ) *= rhs );
}

Matrix4& Matrix4::operator*=( const Matrix4& rhs )
{
	std::array< Vector4, 4 > columns;
	for( size_t i = 0; i < 4; ++i )
		columns[ i ] = Vector4( m_elements[ i * 4 + 0 ], m_elements[ i * 4 + 1 ], m_elements[ i * 4 + 2 ], m_elements[ i * 4 + 3 ] );

	std::array< Vector4, 4 > rows;
	for( size_t i = 0; i < 4; ++i )
		rows[ i ] = Vector4( rhs.m_elements[ 0 * 4 + i ], rhs.m_elements[ 1 * 4 + i ], rhs.m_elements[ 2 * 4 + i ], rhs.m_elements[ 3 * 4 + i ] );

	for( size_t row = 0; row < 4; ++row )
		for( size_t col = 0; col < 4; ++col )
			m_elements[ row * 4 + col ] = columns[ row ].DotProduct( rows[ col ] );

	return *this;
}

float& Matrix4::operator()( size_t column, size_t row )
{
	return m_elements[ row * 4 + column ];
}

const float& Matrix4::operator()( size_t column, size_t row ) const
{
	return m_elements[ row * 4 + column ];
}

ORB_NAMESPACE_END
