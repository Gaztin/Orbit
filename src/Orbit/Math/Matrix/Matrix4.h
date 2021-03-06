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
#include "Orbit/Math/Vector/Vector3.h"
#include "Orbit/Math/Vector/Vector4.h"

#include <array>

ORB_NAMESPACE_BEGIN

class Vector3;

class ORB_API_MATH Matrix4
{
public:

	explicit Matrix4( float diagonal = 1.0f );
	Matrix4         ( std::initializer_list< float > elements );

public:

	void TranslateX    ( float translation );
	void TranslateY    ( float translation );
	void TranslateZ    ( float translation );
	void Translate     ( const Vector3& translation );
	void RotateX       ( float rotation );
	void RotateY       ( float rotation );
	void RotateZ       ( float rotation );
	void Rotate        ( const Vector3& rotation );
	void Scale         ( const Vector3& scale );
	void Transpose     ( void );
	void Invert        ( void );
	void SetIdentity   ( void );
	void SetPerspective( float aspect_ratio, float fov, float near_clip, float far_clip );

public:

	[[ nodiscard ]] float   GetDeterminant   ( void ) const;
	[[ nodiscard ]] float   GetDeterminant3x3( size_t column, size_t row ) const;
	[[ nodiscard ]] Matrix4 Transposed       ( void ) const;
	[[ nodiscard ]] Matrix4 Inverted         ( void ) const;

public:

	Matrix4  operator* ( const Matrix4& rhs ) const;
	Vector4  operator* ( const Vector4& rhs ) const;
	Matrix4& operator*=( const Matrix4& rhs );
	Matrix4& operator= ( const Matrix4& rhs );

public:

	float&       operator[]( size_t i )                        { return reinterpret_cast<       float* >( this )[ i ]; }
	const float& operator[]( size_t i ) const                  { return reinterpret_cast< const float* >( this )[ i ]; }
	float&       operator()( size_t column, size_t row )       { return reinterpret_cast<       float* >( this )[ row * 4 + column ]; }
	const float& operator()( size_t column, size_t row ) const { return reinterpret_cast< const float* >( this )[ row * 4 + column ]; }

public:

	Vector3 right;
	float   pad0;
	Vector3 up;
	float   pad1;
	Vector3 forward;
	float   pad2;
	Vector3 pos;
	float   pad3;

};

ORB_NAMESPACE_END
