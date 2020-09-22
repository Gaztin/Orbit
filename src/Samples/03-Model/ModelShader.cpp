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

#include "ModelShader.h"

#include <Orbit/ShaderGen/Variables/Float.h>
#include <Orbit/ShaderGen/Variables/Vec3.h>
#include <Orbit/ShaderGen/Variables/Vec4.h>

ModelShader::Vec4 ModelShader::VSMain( void )
{
	v_position       = u_view_projection * u_model * a_position;
	v_color          = a_color;
	v_texcoord       = a_texcoord;
	v_normal         = ( u_model * Vec4( a_normal, 0.0f ) )->xyz;
	v_world_position = u_model * a_position;

	return v_position;
}

ModelShader::Vec4 ModelShader::PSMain( void )
{
	constexpr float ambient_luminance = 0.1f;

	Vec4  albedo    = Sample( diffuse_texture, v_texcoord );
	Vec3  ambient   = u_light_color * ambient_luminance;
	Vec3  norm      = Normalize( v_normal );
	Vec3  light_dir = Normalize( u_light_pos - v_world_position->xyz );
	Float NdotL     = Max( Dot( norm, light_dir ), 0.0f );
	Vec3  diffuse   = u_light_color * NdotL;
	Vec3  phong     = ( ambient + diffuse ) * v_color->rgb;

	return Vec4( phong, 1.0f );
}
