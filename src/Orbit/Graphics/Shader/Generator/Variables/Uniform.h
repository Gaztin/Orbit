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
#include "Orbit/Graphics/Shader/Generator/Variables/IVariable.h"

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	class Float;
	class Vec2;
	class Vec3;
	class Vec4;
	class Mat4;

	class ORB_API_GRAPHICS UniformBase : public IVariable
	{
	public:

		UniformBase( DataType type );

	};

	template< typename T >
	class Uniform : public UniformBase
	{
	public:

		Uniform( void );

	};

	template<> inline Uniform< Float >::Uniform( void ) : UniformBase( DataType::Float ) { }
	template<> inline Uniform< Vec2  >::Uniform( void ) : UniformBase( DataType::Vec2  ) { }
	template<> inline Uniform< Vec3  >::Uniform( void ) : UniformBase( DataType::Vec3  ) { }
	template<> inline Uniform< Vec4  >::Uniform( void ) : UniformBase( DataType::Vec4  ) { }
	template<> inline Uniform< Mat4  >::Uniform( void ) : UniformBase( DataType::Mat4  ) { }
}

ORB_NAMESPACE_END