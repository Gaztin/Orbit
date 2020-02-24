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

#include "PostFXShader.h"

#include <Orbit/Graphics/Shader/Generator/Variables/Float.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Vec2.h>

PostFXShader::Vec4 PostFXShader::VSMain( void )
{
	v_texcoord = CanonicalScreenPos( a_position->xy ) * 0.5 + 0.5;
	v_position = a_position;

	return v_position;
}

PostFXShader::Vec4 PostFXShader::PSMain( void )
{
	Vec4 tex_render = Sample( render_texture, v_texcoord );
	Vec4 left       = Sample( render_texture, v_texcoord + Vec2( Cos( u_time ), Sin( u_time ) ) * 0.025 );
	Vec4 right      = Sample( render_texture, v_texcoord - Vec2( Cos( u_time ), Sin( u_time ) ) * 0.025 );

	return tex_render->rgba * 0.5 + left * 0.25 + right * 0.25;
}
