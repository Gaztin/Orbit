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
#include "Orbit/Math/Math.h"

#include <array>

ORB_NAMESPACE_BEGIN

class Vector3;

class ORB_API_MATH Matrix4
{
public:
	explicit Matrix4( float diagonal = 1.f );
	Matrix4( std::initializer_list< float > elements );

	void Translate( const Vector3& t );
	void Rotate( const Vector3& r );
	void Transpose();

	Matrix4 operator*( const Matrix4& rhs ) const;
	Matrix4& operator*=( const Matrix4& rhs );

	float& operator[]( size_t i ) { return m_elements[ i ]; }
	const float& operator[]( size_t i ) const { return m_elements[ i ]; }

private:

	std::array< float, 16 > m_elements;

};

ORB_NAMESPACE_END
