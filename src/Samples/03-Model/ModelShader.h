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
#include <Orbit/Graphics/Shader/VertexLayout.h>

#include <string_view>
#include <tuple>

/* Compile-time int to string by courtesy of https://stackoverflow.com/a/24000041 */

template< uint32_t... Digits >
struct DigitsToChars
{
	static constexpr char value[] = { ( '0' + Digits )..., 0 };
};

template< uint32_t Remainder, uint32_t... Digits >
struct Explode : Explode< Remainder / 10, Remainder % 10, Digits... >
{
};

template< uint32_t... Digits >
struct Explode< 0, Digits... > : DigitsToChars< Digits... >
{
};

template<>
struct Explode< 0 > : DigitsToChars< 0 >
{
};

template< uint32_t Integer >
struct IntToString : Explode< Integer >
{
};

struct ConstexprBuffer
{
	constexpr void Add( std::string_view str )
	{
		for( size_t i = 0; i < str.size(); ++i )
			buf[ size++ ] = str[ i ];
	}

	std::array< char, 4096 > buf;
	size_t                   size;
};

template< typename T, int I >
struct ShaderConstant
{
	static constexpr int index = I;

	template< typename T2 >
	constexpr bool IsSame( void ) const
	{
		return std::is_same_v< T, T2 >;
	}
};

template< typename... Types >
class ShaderConstants
{
public:

	constexpr size_t Size( void ) const
	{
		return sizeof...( Types );
	}

	template< typename Functor, typename... ExtraArgs >
	constexpr void ForEach( Functor func, ExtraArgs&&... extra_args ) const
	{
		Invoke< 0, Functor, Types... >( func, std::forward< ExtraArgs >( extra_args )... );
	}

private:

	template< int Index, typename Functor, typename T, typename... Ts, typename... ExtraArgs >
	constexpr void Invoke( Functor func, ExtraArgs&&... extra_args ) const
	{
		ShaderConstant< T, Index > c{ };
		func( c, std::forward< ExtraArgs >( extra_args )... );

		Invoke< Index + 1, Functor, Ts... >( func, std::forward< ExtraArgs >( extra_args )... );
	}

	template< int Index, typename Functor, typename... ExtraArgs >
	constexpr void Invoke( Functor, ExtraArgs&&... ) const
	{
	}

	template< int Index, typename C, typename T, typename... Ts >
	constexpr int IndexOf( void ) const
	{
		if( std::is_same_v< C, T > )
			return Index;

		return IndexOf< Index + 1, C, Ts... >();
	}

	template< int Index, typename C >
	constexpr int IndexOf( void ) const
	{
		return -1;
	}

};

class ModelShader
{
public:

	constexpr ModelShader( void ) = default;

public:

	constexpr auto      GenerateSourceD3D11 ( void ) const;
	constexpr auto      GenerateSourceOpenGL( void ) const;
	Orbit::VertexLayout GenerateVertexLayout( void ) const;

private:

	static constexpr auto GetSamplers( void )
	{
		return std::array {
			std::string_view( "diffuse_texture" ),
		};
	}

	static constexpr auto GetVertexConstants( void )
	{
		return ShaderConstants<
			Orbit::Matrix4,
			Orbit::Matrix4,
			Orbit::Matrix4
		>{ };
	}

	static constexpr auto GetPixelConstants( void )
	{
		return ShaderConstants<
			Orbit::Vector3
		>{ };
	}

	static constexpr auto GetVertexLayout( void )
	{
		return std::array {
			Orbit::VertexComponent::Position,
			Orbit::VertexComponent::Color,
			Orbit::VertexComponent::TexCoord,
			Orbit::VertexComponent::Normal,
		};
	}

};

constexpr auto ModelShader::GenerateSourceD3D11( void ) const
{
	ConstexprBuffer buffer{ };

	auto samplers = GetSamplers();
	if( !samplers.empty() )
	{
		for( auto sampler : samplers )
		{
			buffer.Add( "Texture2D " );
			buffer.Add( sampler );
			buffer.Add( ";\n" );
		}

		buffer.Add( "SamplerState default_sampler_state;\n" );
	}

	constexpr auto add_variable = []( auto constant, ConstexprBuffer& buffer, std::string_view variable_suffix ) constexpr
	{
		buffer.Add( "\t" );

		if constexpr( constant.IsSame< float >() )
			buffer.Add( "float " );
		else if constexpr( constant.IsSame< Orbit::Vector2 >() )
			buffer.Add( "float2 " );
		else if constexpr( constant.IsSame< Orbit::Vector3 >() )
			buffer.Add( "float3 " );
		else if constexpr( constant.IsSame< Orbit::Vector4 >() )
			buffer.Add( "float4 " );
		else if constexpr( constant.IsSame< Orbit::Matrix4 >() )
			buffer.Add( "matrix " );

		buffer.Add( "reserved_constant" );
		buffer.Add( variable_suffix );
		buffer.Add( "_" );
		buffer.Add( IntToString< constant.index >::value );
		buffer.Add( ";\n" );
	};

	auto vertex_constants = GetVertexConstants();
	if( vertex_constants.Size() != 0 )
	{
		buffer.Add( "cbuffer VertexConstants\n{\n" );
		vertex_constants.ForEach( add_variable, buffer, "_vertex" );
		buffer.Add( "};\n\n" );
	}

	auto pixel_constants = GetPixelConstants();
	if( pixel_constants.Size() != 0 )
	{
		buffer.Add( "cbuffer PixelConstants\n{\n" );
		pixel_constants.ForEach( add_variable, buffer, "_pixel" );
		buffer.Add( "};\n\n" );
	}

	buffer.Add( R"(
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
	output.position = mul( input.position, mul( reserved_constant_vertex_1, reserved_constant_vertex_0 ) );
	output.color    = input.color;
	output.texcoord = input.texcoord;
	output.normal   = mul( float4( input.normal, 1.0 ), transpose( reserved_constant_vertex_2 ) ).xyz;

	return output;
}

float4 PSMain( PixelData input ) : SV_TARGET
{
	float4 tex_color = diffuse_texture.Sample( default_sampler_state, input.texcoord );
	float4 out_color = tex_color + input.color;

	float  diffuse  = -dot( input.normal, reserved_constant_pixel_0 );
	out_color.rgb  *= diffuse;

	return out_color;
}
)" );

	return buffer.buf;
}

constexpr auto ModelShader::GenerateSourceOpenGL( void ) const
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

Orbit::VertexLayout ModelShader::GenerateVertexLayout( void ) const
{
	return Orbit::VertexLayout( GetVertexLayout() );
}
