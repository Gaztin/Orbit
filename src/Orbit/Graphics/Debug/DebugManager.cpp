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
#include "Orbit/Graphics/Geometry/MeshFactory.h"
#include "Orbit/Graphics/Renderer/DefaultRenderer.h"
#include "Orbit/Graphics/Renderer/RenderCommand.h"
#include "Orbit/Math/Matrix4.h"
#include "Orbit/Math/Vector4.h"
#include "Orbit/Core/IO/Log.h"

ORB_NAMESPACE_BEGIN

struct DebugVertex
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
	: shader_               ( shader_source, vertex_layout )
	, line_segments_        { }
	, spheres_              { }
	, lines_vertex_buffer_  ( nullptr, 0, vertex_layout.GetStride(), false )
	, spheres_vertex_buffer_( nullptr, 0, vertex_layout.GetStride(), false )
	, constant_buffer_      ( sizeof( Matrix4 ) )
	, sphere_geometry_      ( MeshFactory::GetInstance().CreateGeometryFromShape( ShapeType::Sphere, vertex_layout ) )
{
}

void DebugManager::PushLineSegment( Vector3 start, Vector3 end, double duration )
{
	LineSegment line_segment;
	line_segment.start   = start;
	line_segment.end     = end;
	line_segment.birth   = Clock::now();
	line_segment.death   = line_segment.birth + std::chrono::duration_cast< Clock::duration >( std::chrono::duration< double >( duration ) );

	line_segments_.emplace_back( std::move( line_segment ) );
}

void DebugManager::PushSphere( Vector3 center, double duration )
{
	Sphere sphere;
	sphere.position = center;
	sphere.birth    = Clock::now();
	sphere.death    = sphere.birth + std::chrono::duration_cast< Clock::duration >( std::chrono::duration< double >( duration ) );

	spheres_.emplace_back( std::move( sphere ) );
}

void DebugManager::Render( IRenderer& renderer, const Matrix4& view_projection )
{
	auto now = Clock::now();

	constant_buffer_.Update( &view_projection, sizeof( Matrix4 ) );

	if( !line_segments_.empty() )
	{
		lines_vertex_buffer_.Update( nullptr, line_segments_.size() * 2 );

		DebugVertex* dst = static_cast< DebugVertex* >( lines_vertex_buffer_.Map() );

		for( const LineSegment& line_segment : line_segments_ )
		{
			const Color color( 1.0f, 0.0f, 0.0f, CalcAlphaForObject( line_segment, now ) );

			dst->position = Vector4( line_segment.start,  1.0f );
			dst->color    = color;
			++dst;

			dst->position = Vector4( line_segment.end, 1.0f );
			dst->color    = color;
			++dst;
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

	if( !spheres_.empty() )
	{
		spheres_vertex_buffer_.Update( nullptr, spheres_.size() * sphere_geometry_.GetFaceCount() * 6 );

		DebugVertex* dst = static_cast< DebugVertex* >( spheres_vertex_buffer_.Map() );

		for( const Sphere& sphere : spheres_ )
		{
			const Color color( 0.0f, 1.0f, 0.0f, CalcAlphaForObject( sphere, now ) );

			for( Face face : sphere_geometry_.GetFaces() )
			{
				Vertex v1 = sphere_geometry_.GetVertex( face.indices[ 0 ] );
				Vertex v2 = sphere_geometry_.GetVertex( face.indices[ 1 ] );
				Vertex v3 = sphere_geometry_.GetVertex( face.indices[ 2 ] );

				v1.position += Vector4( sphere.position, 0.0f );
				v2.position += Vector4( sphere.position, 0.0f );
				v3.position += Vector4( sphere.position, 0.0f );

				dst->position = v1.position;
				dst->color    = color;
				++dst;
				dst->position = v2.position;
				dst->color    = color;
				++dst;

				dst->position = v1.position;
				dst->color    = color;
				++dst;
				dst->position = v3.position;
				dst->color    = color;
				++dst;

				dst->position = v2.position;
				dst->color    = color;
				++dst;
				dst->position = v3.position;
				dst->color    = color;
				++dst;
			}
		}

		spheres_vertex_buffer_.Unmap();

		// Create render command
		RenderCommand render_command;
		render_command.shader        = shader_;
		render_command.vertex_buffer = spheres_vertex_buffer_;
		render_command.topology      = Topology::Lines;
		render_command.constant_buffers[ ShaderType::Vertex ].emplace_back( constant_buffer_ );

		renderer.PushCommand( std::move( render_command ) );
	}
}

void DebugManager::Flush( void )
{
	auto now = Clock::now();

	for( LineSegmentVector::iterator it = line_segments_.begin(); it != line_segments_.end(); )
	{
		auto time_left = ( it->death - now );

		if( time_left < time_left.zero() ) it = line_segments_.erase( it );
		else                               it++;
	}

	for( SphereVector::iterator it = spheres_.begin(); it != spheres_.end(); )
	{
		auto time_left = ( it->death - now );

		if( time_left < time_left.zero() ) it = spheres_.erase( it );
		else                               it++;
	}
}

float DebugManager::CalcAlphaForObject( const DebugObjectBase& object, Clock::time_point now )
{
	// Single-frame objects should always be visible
	if( object.birth == object.death )
		return 1.0f;

	float time_alive        = std::chrono::duration_cast< std::chrono::duration< float > >( now - object.birth ).count();
	float time_left_to_live = std::chrono::duration_cast< std::chrono::duration< float > >( object.death - now ).count();

	return ( time_left_to_live / std::min( time_alive + time_left_to_live, 1.0f ) );
}

ORB_NAMESPACE_END
