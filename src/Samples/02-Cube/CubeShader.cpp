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

#include "CubeShader.h"

#include <Orbit/Graphics/Shader/Generator/Variables/Float.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Vec3.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Vec4.h>

CubeShader::Vec4 CubeShader::VSMain( void )
{
	v_position = u_view_projection * u_model * a_position;
	v_normal   = ( Transpose( u_model_inverse ) * Vec4( a_normal, 0.0 ) )->xyz;
	v_texcoord = a_texcoord;

	return v_position;
}

CubeShader::Vec4 CubeShader::PSMain( void )
{
	Vec4  tex_color = Sample( diffuse_texture, v_texcoord );
	Vec3  light_dir = Vec3( -0.3, 1.0, -0.8 );
	Float influence = ( Dot( v_normal, light_dir ) * 0.5 + 0.5 );

	return tex_color * influence;
}
