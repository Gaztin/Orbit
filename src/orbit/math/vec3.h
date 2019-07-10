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
	class vec3 final : public vec_base< 3, vec3 >
	{
	public:
		vec3()
			: x( 0.f )
			, y( 0.f )
			, z( 0.f )
		{
		}

		explicit vec3( float scalar )
			: x( scalar )
			, y( scalar )
			, z( scalar )
		{
		}

		vec3( float x, float y, float z )
			: x( x )
			, y( y )
			, z( z )
		{
		}

		vec3 cross_product( const vec3& v ) const
		{
			return vec3( ( y * v.z ) - ( z * v.y ),
			             ( z * v.x ) - ( x * v.z ),
			             ( x * v.y ) - ( y * v.x )
			);
		}

		float x;
		float y;
		float z;

	};
}
