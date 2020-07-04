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

#include "DebugManager.h"

#include "Orbit/Core/Utility/Color.h"
#include "Orbit/Graphics/Renderer/DefaultRenderer.h"
#include "Orbit/Graphics/Renderer/RenderCommand.h"
#include "Orbit/Math/Matrix4.h"
#include "Orbit/Math/Vector4.h"

ORB_NAMESPACE_BEGIN

struct Vertex
{
	Vector4 position;
	Color   color;
};

constexpr std::string_view shader_source = R"(
#if defined( HLSL )

cbuffer VertexUniforms
{
	matrix u_view_projection;
};

struct VertexData
{
	float4 position : POSITION;
	float4 color    : COLOR;
};

struct PixelData
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
};

PixelData VSMain( VertexData input )
{
	PixelData output;
	output.position = mul( input.position, u_view_projection );
	output.color    = input.color;

	return output;
}

float4 PSMain( PixelData input ) : SV_TARGET
{
	return input.color;
}

#elif defined( GLSL ) // HLSL

#if defined( VERTEX )

ORB_CONSTANTS_BEGIN( VertexUniforms )
	ORB_CONSTANT( mat4, u_view_projection );
ORB_CONSTANTS_END

ORB_ATTRIBUTE( 0 ) vec4 a_position;
ORB_ATTRIBUTE( 1 ) vec4 a_color;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;

void main()
{
	v_position = u_view_projection * a_position;
	v_color    = a_color;

	gl_Position = v_position;
}

#elif defined( FRAGMENT ) // VERTEX

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;

void main()
{
	ORB_SET_OUT_COLOR( v_color );
}

#endif // FRAGMENT

#endif // GLSL
)";

static VertexLayout vertex_layout
{
	VertexComponent::Position,
	VertexComponent::Color,
};

DebugManager::DebugManager( void )
	: shader_             ( shader_source, vertex_layout )
	, lines_              { }
	, lines_vertex_buffer_( nullptr, 0, vertex_layout.GetStride(), false )
	, constant_buffer_    ( sizeof( Matrix4 ) )
{
}

void DebugManager::PushLineSegment( Vector3 start, Vector3 end )
{
	lines_.emplace_back( std::pair( std::move( start ), std::move( end ) ) );
}

void DebugManager::Render( IRenderer& renderer, const Matrix4& view_projection )
{
	constant_buffer_.Update( &view_projection, sizeof( Matrix4 ) );

	if( !lines_.empty() )
	{
		lines_vertex_buffer_.Update( nullptr, lines_.size() * 2 );

		Vertex* dst = static_cast< Vertex* >( lines_vertex_buffer_.Map() );

		for( size_t i = 0; i < lines_.size(); ++i )
		{
			dst[ i * 2 + 0 ].position = Vector4( lines_[ i ].first,  1.0f );
			dst[ i * 2 + 0 ].color    = Color( 1.0f, 0.0f, 0.0f );
			dst[ i * 2 + 1 ].position = Vector4( lines_[ i ].second, 1.0f );
			dst[ i * 2 + 1 ].color    = Color( 1.0f, 0.0f, 0.0f );
		}

		lines_vertex_buffer_.Unmap();

		// Create render command
		RenderCommand command;
		command.shader        = shader_;
		command.vertex_buffer = lines_vertex_buffer_;
		command.topology      = Topology::Lines;
		command.constant_buffers[ ShaderType::Vertex ].emplace_back( constant_buffer_ );

		renderer.PushCommand( std::move( command ) );
	}
}

void DebugManager::Flush( void )
{
	lines_.clear();
}

ORB_NAMESPACE_END
