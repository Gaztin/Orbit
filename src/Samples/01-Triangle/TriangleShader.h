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
#include <Orbit/Graphics/Shader/Generator/Variables/Attribute.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Sampler.h>
#include <Orbit/Graphics/Shader/Generator/Variables/Varying.h>
#include <Orbit/Graphics/Shader/Generator/IGenerator.h>

class TriangleShader final : public Orbit::ShaderGen::IGenerator
{
public:

	TriangleShader( void ) = default;

private:

	Orbit::ShaderGen::Vec4 VSMain( void ) override;
	Orbit::ShaderGen::Vec4 PSMain( void ) override;

private:

	Orbit::ShaderGen::Sampler diffuse_texture;

	Orbit::ShaderGen::Attribute::Position a_position;
	Orbit::ShaderGen::Attribute::Color    a_color;
	Orbit::ShaderGen::Attribute::TexCoord a_texcoord;

	Orbit::ShaderGen::Varying::Position v_position;
	Orbit::ShaderGen::Varying::Color    v_color;
	Orbit::ShaderGen::Varying::TexCoord v_texcoord;

};
