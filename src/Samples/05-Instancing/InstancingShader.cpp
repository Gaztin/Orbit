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

#include "InstancingShader.h"

#include <Orbit/Graphics/Shader/Generator/Variables/Float.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Vec4.h>

InstancingShader::Vec4 InstancingShader::VSMain( void )
{
	v_position = u_view_projection * u_model * a_position;
	v_color    = a_color;
	v_texcoord = a_texcoord;
	v_normal   = ( Transpose( u_model_inverse ) * Vec4( a_normal, 1.0 ) )->xyz;

	return v_position;
}

InstancingShader::Vec4 InstancingShader::PSMain( void )
{
	Vec4 tex_color = Sample( diffuse_texture, v_texcoord );
	Vec4 out_color = tex_color + v_color;

	Float diffuse  = ( -Dot( v_normal, u_light_dir ) * 0.5 );
	out_color->rgb *= diffuse;

	return out_color;
}
