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
#include "Orbit/Graphics/Context/RenderContext.h"
#include "Orbit/Graphics/Shader/ShaderInterface.h"

#include <string_view>

class ModelShader
{
public:

	constexpr ModelShader( void ) = default;

public:

	constexpr std::string_view GenerateSourceD3D11 ( void ) const;
	constexpr std::string_view GenerateSourceOpenGL( void ) const;

};

constexpr std::string_view ModelShader::GenerateSourceD3D11( void ) const
{
	return R"(
Texture2D diffuse_texture;

SamplerState texture_sampler;

cbuffer VertexConstants
{
	matrix view_projection;
	matrix model;
	matrix model_inverse;
};

cbuffer PixelConstants
{
	float3 light_dir;
};

struct VertexData
{
	float4 position : POSITION;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
	float3 normal   : NORMAL;
};

struct PixelData
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
	float3 normal   : NORMAL;
};

PixelData VSMain( VertexData input )
{
	PixelData output;
	output.position = mul( input.position, mul( model, view_projection ) );
	output.color    = input.color;
	output.texcoord = input.texcoord;
	output.normal   = mul( float4( input.normal, 1.0 ), transpose( model_inverse ) ).xyz;

	return output;
}

float4 PSMain( PixelData input ) : SV_TARGET
{
	float4 tex_color = diffuse_texture.Sample( texture_sampler, input.texcoord );
	float4 out_color = tex_color + input.color;

	float  diffuse  = -dot( input.normal, light_dir );
	out_color.rgb  *= diffuse;

	return out_color;
}
)";
}

constexpr std::string_view ModelShader::GenerateSourceOpenGL( void ) const
{
	return R"(
#if defined( VERTEX )

ORB_CONSTANTS_BEGIN( VertexConstants )
	ORB_CONSTANT( mat4, view_projection );
	ORB_CONSTANT( mat4, model );
	ORB_CONSTANT( mat4, model_inverse );
ORB_CONSTANTS_END

ORB_ATTRIBUTE( 0 ) vec4 a_position;
ORB_ATTRIBUTE( 1 ) vec4 a_color;
ORB_ATTRIBUTE( 2 ) vec2 a_texcoord;
ORB_ATTRIBUTE( 3 ) vec3 a_normal;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;
ORB_VARYING vec3 v_normal;

void main()
{
	v_position = view_projection * model * a_position;
	v_color    = a_color;
	v_texcoord = a_texcoord;
	v_normal   = ( transpose( model_inverse ) * vec4( a_normal, 1.0 ) ).xyz;

	gl_Position = v_position;
}

#  elif defined( FRAGMENT )

ORB_CONSTANTS_BEGIN( FragmentConstants )
	ORB_CONSTANT( vec3, light_dir );
ORB_CONSTANTS_END

uniform sampler2D diffuse_texture;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;
ORB_VARYING vec3 v_normal;

void main()
{
	vec4 tex_color = texture( diffuse_texture, v_texcoord );
	vec4 out_color = tex_color + v_color;

	float diffuse  = -dot( v_normal, light_dir );
	out_color.rgb *= diffuse;

	ORB_SET_OUT_COLOR( out_color );
}

#endif
)";
}
