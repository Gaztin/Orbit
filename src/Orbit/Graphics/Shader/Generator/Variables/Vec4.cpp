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

#include "Vec4.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	Vec4::Vec4( const IVariable& a )
		: IVariable( a )
	{
		assert( a.GetType() == VariableType::Vec4 );

		a.SetUsed();
	}

	Vec4::Vec4( const IVariable& a, const IVariable& b )
		: IVariable( "vec4( " + a.GetValue() + ", " + b.GetValue() + " )", VariableType::Vec4 )
	{
		assert( ( ( a.GetType() == VariableType::Vec3  ) && ( b.GetType() == VariableType::Float ) ) ||
		        ( ( a.GetType() == VariableType::Vec2  ) && ( b.GetType() == VariableType::Vec2  ) ) ||
		        ( ( a.GetType() == VariableType::Float ) && ( b.GetType() == VariableType::Vec3  ) ) );

		a.SetUsed();
		b.SetUsed();
	}

	Vec4::Vec4( const IVariable& a, const IVariable& b, const IVariable& c )
		: IVariable( "vec4( " + a.GetValue() + ", " + b.GetValue() + ", " + c.GetValue() + " )", VariableType::Vec4 )
	{
		assert( ( ( a.GetType() == VariableType::Vec2  ) && ( b.GetType() == VariableType::Float ) && ( c.GetType() == VariableType::Float ) ) ||
		        ( ( a.GetType() == VariableType::Float ) && ( b.GetType() == VariableType::Vec2  ) && ( c.GetType() == VariableType::Float ) ) ||
		        ( ( a.GetType() == VariableType::Float ) && ( b.GetType() == VariableType::Float ) && ( c.GetType() == VariableType::Vec2  ) ) );

		a.SetUsed();
		b.SetUsed();
		c.SetUsed();
	}

	Vec4::Vec4( const IVariable& a, const IVariable& b, const IVariable& c, const IVariable& d )
		: IVariable( "vec4( " + a.GetValue() + ", " + b.GetValue() + ", " + c.GetValue() + ", " + d.GetValue() + " )", VariableType::Vec4 )
	{
		assert( ( ( a.GetType() == VariableType::Float ) && ( b.GetType() == VariableType::Float ) && ( c.GetType() == VariableType::Float ) && ( d.GetType() == VariableType::Float ) ) );

		a.SetUsed();
		b.SetUsed();
		c.SetUsed();
		d.SetUsed();
	}
}

ORB_NAMESPACE_END
