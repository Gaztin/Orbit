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

#pragma once
#include <Orbit/ShaderGen/Generator/IShader.h>
#include <Orbit/ShaderGen/Variables/Attribute.h>
#include <Orbit/ShaderGen/Variables/Sampler.h>
#include <Orbit/ShaderGen/Variables/Uniform.h>
#include <Orbit/ShaderGen/Variables/Varying.h>

class SceneShader final : public Orbit::ShaderGen::IShader
{
public:

	SceneShader( void ) = default;

private:

	Vec4 VSMain( void ) override;
	Vec4 PSMain( void ) override;

private:

	Sampler diffuse_texture;

	Attribute::Position a_position;
	Attribute::Color    a_color;
	Attribute::TexCoord a_texcoord;
	Attribute::Normal   a_normal;

	Varying::Position v_position;
	Varying::Color    v_color;
	Varying::TexCoord v_texcoord;
	Varying::Normal   v_normal;

public:

	Uniform< Mat4 > u_view_projection;
	Uniform< Mat4 > u_model;

};
