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
#include <Orbit/Graphics/Shader/ShaderInterface.h>
#include <Orbit/Graphics/Shader/VertexLayout.h>

class ModelShader final : public Orbit::ShaderInterface
{
public:

	ModelShader( void ) = default;

private:

	Vec4 VSMain( void ) override;
	Vec4 PSMain( void ) override;

private:

	Sampler diffuse_texture;

	Attribute a_position = Orbit::VertexComponent::Position;
	Attribute a_color    = Orbit::VertexComponent::Color;
	Attribute a_texcoord = Orbit::VertexComponent::TexCoord;
	Attribute a_normal   = Orbit::VertexComponent::Normal;

	Varying v_position = Orbit::VertexComponent::Position;
	Varying v_color    = Orbit::VertexComponent::Color;
	Varying v_texcoord = Orbit::VertexComponent::TexCoord;
	Varying v_normal   = Orbit::VertexComponent::Normal;

	Uniform u_view_projection = VariableType::MAT4;
	Uniform u_model           = VariableType::MAT4;
	Uniform u_model_inverse   = VariableType::MAT4;

	Uniform u_light_dir       = VariableType::VEC3;

};
