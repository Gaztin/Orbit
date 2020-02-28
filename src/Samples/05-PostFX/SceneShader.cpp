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

#include "SceneShader.h"

#include <Orbit/Graphics/Shader/Generator/Variables/Float.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Vec3.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Vec4.h>

SceneShader::Vec4 SceneShader::VSMain( void )
{
	v_position = u_view_projection * u_model * a_position;
	v_color    = a_color;
	v_texcoord = a_texcoord;
	v_normal   = a_normal;

	return v_position;
}

SceneShader::Vec4 SceneShader::PSMain( void )
{
	Vec4 tex_color = Sample( diffuse_texture, v_texcoord );
	Vec4 out_color = tex_color + v_color;

	Vec3  light_dir             = Normalize( Vec3( -1.0, 1.0, 1.0 ) );
	Float directional_influence = ( Dot( v_normal, light_dir ) * 0.75 );
	Float ambient_influence     = 0.5;

	out_color->rgb *= ( directional_influence + ambient_influence );

	return out_color;
}
