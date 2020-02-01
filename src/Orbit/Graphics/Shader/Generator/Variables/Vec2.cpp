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

#include "Vec2.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	Vec2::Vec2( const IVariable& a )
		: IVariable( "vec2( " + a.GetValue() + " )", VariableType::Vec2 )
	{
		assert( a.GetType() == VariableType::Vec2 );

		a.SetUsed();
	}
	
	Vec2::Vec2( const IVariable& a, const IVariable& b )
		: IVariable( "vec2( " + a.GetValue() + ", " + b.GetValue() + " )", VariableType::Vec2 )
	{
		assert( ( a.GetType() == VariableType::Float ) && ( b.GetType() == VariableType::Float ) );

		a.SetUsed();
		b.SetUsed();
	}
}

ORB_NAMESPACE_END
