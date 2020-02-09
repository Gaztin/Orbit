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

#include "AnimationShader.h"

#include <Orbit/Graphics/Shader/Generator/Variables/Float.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Vec3.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Vec4.h>

AnimationShader::Vec4 AnimationShader::VSMain( void )
{
	Vec4 total_local_pos = Vec4( 0.0, 0.0, 0.0, 0.0 );
	Vec4 total_normal    = Vec4( 0.0, 0.0, 0.0, 0.0 );

	for( size_t i = 0; i < 4; ++i )
	{
		Vec4 local_position = u_joint_transforms[ a_joint_ids[ i ] ] * a_position;
		total_local_pos    += local_position * a_weights[ i ];

		Vec4 world_normal = u_joint_transforms[ a_joint_ids[ i ] ] * Vec4( a_normal, 0.0 );
		total_normal     += world_normal * a_weights[ i ];
	}

	v_position = u_view_projection * Vec4( total_local_pos->xyz, 1.0 );
	v_color    = a_color;
	v_texcoord = a_texcoord;
	v_normal   = total_normal->xyz;

	return v_position;
}

AnimationShader::Vec4 AnimationShader::PSMain( void )
{
	Vec4 tex_color = Sample( diffuse_texture, v_texcoord );
	Vec4 out_color = tex_color + v_color;

	Float directional_impact = ( -Dot( v_normal, Normalize( Vec3( 0.4, -1.0, 1.0 ) ) ) * 0.75 );
	Float ambient_impact     = 0.5;

	out_color->rgb *= ( directional_impact + ambient_impact );

	return out_color;
}
