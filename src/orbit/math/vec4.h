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
#include "orbit/math/vec_base.h"

namespace orb
{
	class vec4 final : public vec_base< 4, vec4 >
	{
	public:
		vec4()
			: x( 0.f )
			, y( 0.f )
			, z( 0.f )
			, w( 0.f )
		{
		}

		explicit vec4( float scalar )
			: x( scalar )
			, y( scalar )
			, z( scalar )
			, w( scalar )
		{
		}

		vec4( float x, float y, float z, float w )
			: x( x )
			, y( y )
			, z( z )
			, w( w )
		{
		}

		float x;
		float y;
		float z;
		float w;

	};
}