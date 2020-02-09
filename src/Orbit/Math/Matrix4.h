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
#include "Orbit/Math/Vector3.h"
#include "Orbit/Math/Vector4.h"

#include <array>

#if defined( ORB_CC_MSVC )
#  pragma warning( push )
#  pragma warning( disable : 4201 ) // "nonstandard extension used: nameless struct/union"
#endif // ORB_CC_MSVC

ORB_NAMESPACE_BEGIN

class Vector3;

class ORB_API_MATH Matrix4
{
public:

	explicit Matrix4( float diagonal = 1.0f );
	Matrix4         ( std::initializer_list< float > elements );

public:

	void Translate     ( const Vector3& translation );
	void Rotate        ( const Vector3& rotation );
	void Transpose     ( void );
	void Invert        ( void );
	void SetIdentity   ( void );
	void SetPerspective( float aspect_ratio, float fov, float near_clip, float far_clip );

public:

	float GetDeterminant   ( void ) const;
	float GetDeterminant3x3( size_t column, size_t row ) const;

public:

	Matrix4  operator* ( const Matrix4& rhs ) const;
	Vector4  operator* ( const Vector4& rhs ) const;
	Matrix4& operator*=( const Matrix4& rhs );
	Matrix4& operator= ( const Matrix4& rhs );

public:

	float&       operator[]( size_t i )                        { return elements[ i ]; }
	const float& operator[]( size_t i ) const                  { return elements[ i ]; }
	float&       operator()( size_t column, size_t row )       { return elements[ row * 4 + column ]; }
	const float& operator()( size_t column, size_t row ) const { return elements[ row * 4 + column ]; };

public:

	union
	{
		std::array< float, 16 > elements;

		struct
		{
			Vector3 right;
			float   pad0;
			Vector3 up;
			float   pad1;
			Vector3 forward;
			float   pad2;
			Vector3 pos;
			float   pad3;
		};
	};

};

ORB_NAMESPACE_END

#if defined( ORB_CC_MSVC )
#  pragma warning( pop )
#endif // ORB_CC_MSVC
