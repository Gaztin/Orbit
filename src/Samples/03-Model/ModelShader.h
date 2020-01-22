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

#include <array>
#include <string_view>
#include <tuple>

template< size_t I >
struct IndexedKey
{
	static constexpr size_t index = I;
};

template< typename T, size_t I >
struct IndexedPair
{
	static constexpr size_t index = I;

	T value;
};

template< size_t I, size_t N, typename Functor, typename... ExtraArgs >
constexpr void InvokeIndexKeys( Functor&& functor, ExtraArgs&&... extra_args )
{
	functor( IndexedKey< I >{ }, std::forward< ExtraArgs >( extra_args )... );

	if constexpr( I + 1 < N )
		InvokeIndexKeys< I + 1, N, Functor >( std::forward< Functor >( functor ), std::forward< ExtraArgs >( extra_args )... );
}

template< size_t I, typename T, size_t N, typename Functor, typename... ExtraArgs >
constexpr void InvokeIndexPairs( const std::array< T, N >& arr, Functor&& functor, ExtraArgs&&... extra_args )
{
	functor( IndexedPair< T, I >{ arr[ I ] }, std::forward< ExtraArgs >( extra_args )... );

	if constexpr( I + 1 < N )
		InvokeIndexPairs< I + 1, T, N, Functor >( arr, std::forward< Functor >( functor ), std::forward< ExtraArgs >( extra_args )... );
}

template< typename T, size_t N, typename Functor, typename... ExtraArgs >
constexpr void ArrayForEach( const std::array< T, N >& arr, Functor&& functor, ExtraArgs&&... extra_args )
{
	if constexpr( N > 0 )
		InvokeIndexPairs< 0, T, N, Functor >( arr, std::forward< Functor >( functor ), std::forward< ExtraArgs >( extra_args )... );
}

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
	constexpr void ForEach( Functor&& func, ExtraArgs&&... extra_args ) const
	{
		Invoke< 0, Functor, Types... >( std::forward< Functor >( func ), std::forward< ExtraArgs >( extra_args )... );
	}

private:

	template< int Index, typename Functor, typename T, typename... Ts, typename... ExtraArgs >
	constexpr void Invoke( Functor&& func, ExtraArgs&&... extra_args ) const
	{
		ShaderConstant< T, Index > c{ };
		func( c, std::forward< ExtraArgs >( extra_args )... );

		Invoke< Index + 1, Functor, Ts... >( std::forward< Functor >( func ), std::forward< ExtraArgs >( extra_args )... );
	}

	template< int Index, typename Functor, typename... ExtraArgs >
	constexpr void Invoke( Functor&&, ExtraArgs&&... ) const
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

	static constexpr size_t GetSamplerCount( void )
	{
		return 1;
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

	constexpr size_t sampler_count = GetSamplerCount();
	if constexpr( sampler_count > 0 )
	{
		constexpr auto add_sampler = []( auto it, ConstexprBuffer& buffer ) constexpr
		{
			buffer.Add( "Texture2D sampler_" );
			buffer.Add( IntToString< it.index >::value );
			buffer.Add( ";\n" );
		};

		InvokeIndexKeys< 0, sampler_count >( add_sampler, buffer );

		buffer.Add( "\nSamplerState default_sampler_state;\n" );
	}

	constexpr auto add_constant = []( auto constant, ConstexprBuffer& buffer, std::string_view variable_suffix ) constexpr
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
	if constexpr( vertex_constants.Size() != 0 )
	{
		buffer.Add( "\ncbuffer VertexConstants\n{\n" );
		vertex_constants.ForEach( add_constant, buffer, "_vertex" );
		buffer.Add( "};\n" );
	}

	auto pixel_constants = GetPixelConstants();
	if constexpr( pixel_constants.Size() != 0 )
	{
		buffer.Add( "\ncbuffer PixelConstants\n{\n" );
		pixel_constants.ForEach( add_constant, buffer, "_pixel" );
		buffer.Add( "};\n" );
	}

	auto vertex_layout = GetVertexLayout();
	if constexpr( !vertex_layout.empty() )
	{
		/* VertexData */
		{
			buffer.Add( "\nstruct VertexData\n{\n" );

			for( Orbit::VertexComponent component : vertex_layout )
			{
				switch( component )
				{
					case Orbit::VertexComponent::Position: { buffer.Add( "\tfloat4 position : POSITION;\n" ); } break;
					case Orbit::VertexComponent::Normal:   { buffer.Add( "\tfloat3 normal   : NORMAL;\n" );   } break;
					case Orbit::VertexComponent::Color:    { buffer.Add( "\tfloat4 color    : COLOR;\n" );    } break;
					case Orbit::VertexComponent::TexCoord: { buffer.Add( "\tfloat2 texcoord : TEXCOORD;\n" ); } break;
				}
			}

			buffer.Add( "};\n" );
		}

		/* PixelData */
		{
			buffer.Add( "\nstruct PixelData\n{\n" );

			for( Orbit::VertexComponent component : vertex_layout )
			{
				switch( component )
				{
					case Orbit::VertexComponent::Position: { buffer.Add( "\tfloat4 position : SV_POSITION;\n" ); } break;
					case Orbit::VertexComponent::Normal:   { buffer.Add( "\tfloat3 normal   : NORMAL;\n" );      } break;
					case Orbit::VertexComponent::Color:    { buffer.Add( "\tfloat4 color    : COLOR;\n" );       } break;
					case Orbit::VertexComponent::TexCoord: { buffer.Add( "\tfloat2 texcoord : TEXCOORD;\n" );    } break;
				}
			}

			buffer.Add( "};\n" );
		}
	}

	buffer.Add( R"(
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
	float4 tex_color = sampler_0.Sample( default_sampler_state, input.texcoord );
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
	ConstexprBuffer buffer{ };

	constexpr auto add_constant = []( auto constant, ConstexprBuffer& buffer, std::string_view variable_suffix ) constexpr
	{
		buffer.Add( "\tORB_CONSTANT( " );

		if constexpr( constant.IsSame< float >() )
			buffer.Add( "float" );
		else if constexpr( constant.IsSame< Orbit::Vector2 >() )
			buffer.Add( "vec2" );
		else if constexpr( constant.IsSame< Orbit::Vector3 >() )
			buffer.Add( "vec3" );
		else if constexpr( constant.IsSame< Orbit::Vector4 >() )
			buffer.Add( "vec4" );
		else if constexpr( constant.IsSame< Orbit::Matrix4 >() )
			buffer.Add( "mat4" );

		buffer.Add( ", reserved_constant" );
		buffer.Add( variable_suffix );
		buffer.Add( "_" );
		buffer.Add( IntToString< constant.index >::value );
		buffer.Add( " );\n" );
	};

	/* Attributes */
	constexpr auto add_attribute = []( auto it, ConstexprBuffer& buffer ) constexpr
	{
		buffer.Add( "ORB_ATTRIBUTE( " );
		buffer.Add( IntToString< it.index >::value );
		buffer.Add( " ) " );

		switch( it.value )
		{
			case Orbit::VertexComponent::Position: { buffer.Add( "vec4 a_position;\n" ); } break;
			case Orbit::VertexComponent::Normal:   { buffer.Add( "vec3 a_normal;\n" );   } break;
			case Orbit::VertexComponent::Color:    { buffer.Add( "vec4 a_color;\n" );    } break;
			case Orbit::VertexComponent::TexCoord: { buffer.Add( "vec2 a_texcoord;\n" ); } break;
		}
	};

	/* Varyings */
	constexpr auto add_varying = []( auto it, ConstexprBuffer& buffer ) constexpr
	{
		buffer.Add( "ORB_VARYING " );

		switch( it.value )
		{
			case Orbit::VertexComponent::Position: { buffer.Add( "vec4 v_position;\n" ); } break;
			case Orbit::VertexComponent::Normal:   { buffer.Add( "vec3 v_normal;\n" );   } break;
			case Orbit::VertexComponent::Color:    { buffer.Add( "vec4 v_color;\n" );    } break;
			case Orbit::VertexComponent::TexCoord: { buffer.Add( "vec2 v_texcoord;\n" ); } break;
		}
	};

	auto vertex_layout = GetVertexLayout();

	/* Vertex shader */
	{
		buffer.Add( "#if defined( VERTEX )\n" );

		/* Vertex constants*/
		auto vertex_constants = GetVertexConstants();
		if constexpr( vertex_constants.Size() )
		{
			buffer.Add( "\nORB_CONSTANTS_BEGIN( VertexConstants )\n" );
			vertex_constants.ForEach( add_constant, buffer, "_vertex" );
			buffer.Add( "ORB_CONSTANTS_END\n" );
		}

		/* Attributes and varying variables */
		if constexpr( !vertex_layout.empty() )
		{
			buffer.Add( "\n" );
			ArrayForEach( vertex_layout, add_attribute, buffer );
			buffer.Add( "\n" );
			ArrayForEach( vertex_layout, add_varying, buffer );
		}
	}

	/* Pixel shader */
	{
		buffer.Add( "\n#elif defined( FRAGMENT )\n" );

		/* Pixel constants */
		auto pixel_constants = GetPixelConstants();
		if constexpr( pixel_constants.Size() )
		{
			buffer.Add( "\nORB_CONSTANTS_BEGIN( PixelConstants )\n" );
			pixel_constants.ForEach( add_constant, buffer, "_pixel" );
			buffer.Add( "ORB_CONSTANTS_END\n" );
		}

		/* Samplers */
		constexpr size_t sampler_count = GetSamplerCount();
		if constexpr( sampler_count > 0 )
		{
			constexpr auto add_sampler = []( auto it, ConstexprBuffer& buffer ) constexpr
			{
				buffer.Add( "uniform sampler2D sampler_" );
				buffer.Add( IntToString< it.index >::value );
				buffer.Add( ";\n" );
			};

			buffer.Add( "\n" );
			InvokeIndexKeys< 0, sampler_count >( add_sampler, buffer );
		}

		/* Varying variables */
		if constexpr( !vertex_layout.empty() )
		{
			buffer.Add( "\n" );
			ArrayForEach( vertex_layout, add_varying, buffer );
		}
	}

	buffer.Add( "\n#endif\n" );
	buffer.Add( R"(
#if defined( VERTEX )

void main()
{
	v_position = reserved_constant_vertex_0 * reserved_constant_vertex_1 * a_position;
	v_color    = a_color;
	v_texcoord = a_texcoord;
	v_normal   = ( transpose( reserved_constant_vertex_2 ) * vec4( a_normal, 1.0 ) ).xyz;

	gl_Position = v_position;
}

#elif defined( FRAGMENT )

void main()
{
	vec4 tex_color = texture( sampler_0, v_texcoord );
	vec4 out_color = tex_color + v_color;

	float diffuse  = -dot( v_normal, reserved_constant_pixel_0 );
	out_color.rgb *= diffuse;

	ORB_SET_OUT_COLOR( out_color );
}

#endif
)" );

	return buffer.buf;
}

Orbit::VertexLayout ModelShader::GenerateVertexLayout( void ) const
{
	return Orbit::VertexLayout( GetVertexLayout() );
}
