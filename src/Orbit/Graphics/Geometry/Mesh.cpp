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

#include "Mesh.h"

#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Context/RenderContext.h"
#include "Orbit/Graphics/Geometry/Face.h"
#include "Orbit/Graphics/Geometry/Geometry.h"
#include "Orbit/Math/Geometry/LineSegment.h"
#include "Orbit/Math/Geometry/Plane.h"
#include "Orbit/Math/Geometry/Triangle3D.h"

#include <cassert>

// #FIXME: For debugging purposes
#include "Orbit/Graphics/Debug/DebugManager.h"

ORB_NAMESPACE_BEGIN

constexpr double debug_duration = 15.0;

Mesh::Mesh( std::string_view name )
	: name_( name )
{
}

Geometry Mesh::ToGeometry( void ) const
{
	Geometry geometry( vertex_layout_ );
	auto&    vb_details = vertex_buffer_->GetDetails();

	switch( vb_details.index() )
	{

#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_VertexBufferDetailsD3D11, Private::VertexBufferDetails > ):
		{
			auto&                    context  = std::get< Private::_RenderContextDetailsD3D11 >( RenderContext::GetInstance().GetPrivateDetails() );
			auto&                    vb_d3d11 = std::get< Private::_VertexBufferDetailsD3D11 >( vb_details );
			D3D11_BUFFER_DESC        temp_vb_desc;
			D3D11_MAPPED_SUBRESOURCE temp_vb_mapped;
			ComPtr< ID3D11Buffer >   temp_vb;

			vb_d3d11.buffer->GetDesc( &temp_vb_desc );
			temp_vb_desc.Usage          = D3D11_USAGE_DEFAULT;
			temp_vb_desc.BindFlags      = D3D11_BIND_UNORDERED_ACCESS;
			temp_vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			context.device->CreateBuffer( &temp_vb_desc, nullptr, &temp_vb.ptr_ );
			context.device_context->CopyResource( temp_vb.ptr_, vb_d3d11.buffer.ptr_ );
			context.device_context->Map( temp_vb.ptr_, 0, D3D11_MAP_READ, 0, &temp_vb_mapped );

//////////////////////////////////////////////////////////////////////////

			if( index_buffer_ )
			{
				auto&                    ib_d3d11 = std::get< Private::_IndexBufferDetailsD3D11 >( index_buffer_->GetDetails() );
				D3D11_BUFFER_DESC        temp_ib_desc;
				D3D11_MAPPED_SUBRESOURCE temp_ib_mapped;
				ComPtr< ID3D11Buffer >   temp_ib;

				ib_d3d11.buffer->GetDesc( &temp_ib_desc );
				temp_ib_desc.Usage          = D3D11_USAGE_DEFAULT;
				temp_ib_desc.BindFlags      = D3D11_BIND_UNORDERED_ACCESS;
				temp_ib_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

				context.device->CreateBuffer( &temp_ib_desc, nullptr, &temp_ib.ptr_ );
				context.device_context->CopyResource( temp_ib.ptr_, ib_d3d11.buffer.ptr_ );
				context.device_context->Map( temp_ib.ptr_, 0, D3D11_MAP_READ, 0, &temp_ib_mapped );

				// Supply geometry with vertex and index data
				geometry.SetFromData( { static_cast< const uint8_t* >( temp_vb_mapped.pData ), vertex_buffer_->GetTotalSize() }, { static_cast< const uint8_t* >( temp_ib_mapped.pData ), index_buffer_->GetSize() }, index_buffer_->GetFormat() );

				context.device_context->Unmap( temp_ib.ptr_, 0 );
			}
			else
			{
				// Supply geometry with just vertex data
				geometry.SetFromData( { static_cast< const uint8_t* >( temp_vb_mapped.pData ), temp_vb_desc.ByteWidth } );
			}

			context.device_context->Unmap( temp_vb.ptr_, 0 );

		} break;

#endif // ORB_HAS_D3D11
#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_VertexBufferDetailsOpenGL, Private::VertexBufferDetails > ):
		{
			auto&        vb_opengl = std::get< Private::_VertexBufferDetailsOpenGL >( vb_details );
			const size_t vb_size   = ( vertex_buffer_->GetCount() * vertex_layout_.GetStride() );

			glBindBuffer( OpenGLBufferTarget::Array, vb_opengl.id );
			const void*  vb_src    = glMapBufferRange( OpenGLBufferTarget::Array, 0, vb_size, OpenGLMapAccess::ReadBit );

			if( index_buffer_ )
			{
				auto&        ib_opengl = std::get< Private::_IndexBufferDetailsOpenGL >( index_buffer_->GetDetails() );
				const size_t ib_size   = index_buffer_->GetSize();

				glBindBuffer( OpenGLBufferTarget::ElementArray, ib_opengl.id );
				const void*  ib_src    = glMapBufferRange( OpenGLBufferTarget::ElementArray, 0, ib_size, OpenGLMapAccess::ReadBit );

				// Supply geometry with vertex and index data
				geometry.SetFromData( { static_cast< const uint8_t* >( vb_src ), vb_size }, { static_cast< const uint8_t* >( ib_src ), ib_size }, index_buffer_->GetFormat() );

				glUnmapBuffer( OpenGLBufferTarget::ElementArray );
				glBindBuffer( OpenGLBufferTarget::ElementArray, 0 );
			}
			else
			{
				// Supply geometry with just vertex data
				geometry.SetFromData( { static_cast< const uint8_t* >( vb_src ), vb_size } );
			}

			glUnmapBuffer( OpenGLBufferTarget::Array );
			glBindBuffer( OpenGLBufferTarget::Array, 0 );

		} break;

#endif // ORB_HAS_OPENGL

	}

	return geometry;
}

std::vector< Mesh > Mesh::Slice( const Plane& plane ) const
{
	std::vector< Mesh > meshes;

	if( vertex_buffer_ && vertex_layout_.Contains( VertexComponent::Position ) )
	{
		// #TODO: Support non-indexed geometry
		if( !index_buffer_ )
			return { };

//////////////////////////////////////////////////////////////////////////

		const size_t               vertex_stride     = vertex_layout_.GetStride();
		const size_t               pos_offset        = vertex_layout_.OffsetOf( VertexComponent::Position );
		const size_t               normal_offset     = vertex_layout_.OffsetOf( VertexComponent::Normal );
		const size_t               color_offset      = vertex_layout_.OffsetOf( VertexComponent::Color );
		const size_t               texcoord_offset   = vertex_layout_.OffsetOf( VertexComponent::TexCoord );
		const Geometry             geometry          = ToGeometry();
		Geometry                   geometry_positive( vertex_layout_ );
		Geometry                   geometry_negative( vertex_layout_ );
		std::vector< LineSegment > all_seams;

		for( auto face : geometry.GetFaces() )
		{
			Vertex src_vertices[ 3 ]
			{
				geometry.GetVertex( face.indices[ 0 ] ),
				geometry.GetVertex( face.indices[ 1 ] ),
				geometry.GetVertex( face.indices[ 2 ] ),
			};

			// Take transform into account when slicing
			src_vertices[ 0 ].position = transform_ * src_vertices[ 0 ].position;
			src_vertices[ 1 ].position = transform_ * src_vertices[ 1 ].position;
			src_vertices[ 2 ].position = transform_ * src_vertices[ 2 ].position;

			// Edges are in a specific sequence so that 'edges[x]' is the opposite edge of the corner 'src_verticies[x]'
			const LineSegment edges[ 3 ]
			{
				LineSegment( Vector3( src_vertices[ 1 ].position ), Vector3( src_vertices[ 2 ].position ) ),
				LineSegment( Vector3( src_vertices[ 2 ].position ), Vector3( src_vertices[ 0 ].position ) ),
				LineSegment( Vector3( src_vertices[ 0 ].position ), Vector3( src_vertices[ 1 ].position ) ),
			};

			struct VertexTriangle
			{
				Vertex vertices[ 3 ];
			};

//////////////////////////////////////////////////////////////////////////

			std::array< Vector3, 3 > intersections;
			size_t                   secluded_vertex_index = 0;
			size_t                   intersection_count    = 0;

			for( size_t i = 0; i < 3; ++i )
			{
				if( auto intersection = plane.Intersect( edges[ i ] ); intersection.index() == 1 )
				{
					intersections[ i ] = std::move( std::get< 1 >( intersection ) );
					++intersection_count;
				}
				else
				{
					secluded_vertex_index = i;
				}
			}

			// Two intersections means the plane cuts the triangle in half.
			if( intersection_count == 2 )
			{
				LineSegment seam;

				switch( secluded_vertex_index )
				{
					case 0: { seam = LineSegment( intersections[ 1 ], intersections[ 2 ] ); } break;
					case 1: { seam = LineSegment( intersections[ 0 ], intersections[ 2 ] ); } break;
					case 2: { seam = LineSegment( intersections[ 0 ], intersections[ 1 ] ); } break;
				}

				all_seams.push_back( seam );

//////////////////////////////////////////////////////////////////////////

				std::array< Vertex, 3 > intersection_vertices
				{
					src_vertices[ secluded_vertex_index ],
					src_vertices[ secluded_vertex_index ],
					src_vertices[ secluded_vertex_index ],
				};

				for( size_t i = 0; i < 3; ++i )
				{
					intersection_vertices[ i ].position  = Vector4( intersections[ i ], 1.0f );
					intersection_vertices[ i ].tex_coord = ( src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].tex_coord + ( ( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].tex_coord - src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].tex_coord ) * ( LineSegment( intersections[ 0 ], Vector3( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].position ) ).Length() / LineSegment( Vector3( src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].position ), Vector3( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].position ) ).Length() ) ) );
				}

//////////////////////////////////////////////////////////////////////////

				VertexTriangle secluded_triangle;
				VertexTriangle paired_triangles[ 2 ];

				secluded_triangle.vertices[ 0 ] = src_vertices[ secluded_vertex_index ];
				secluded_triangle.vertices[ 1 ] = intersection_vertices[ ( secluded_vertex_index + 2 ) % 3 ];
				secluded_triangle.vertices[ 2 ] = intersection_vertices[ ( secluded_vertex_index + 1 ) % 3 ];

				paired_triangles[ 0 ].vertices[ 0 ] = src_vertices[ ( secluded_vertex_index + 1 ) % 3 ];
				paired_triangles[ 0 ].vertices[ 1 ] = intersection_vertices[ ( secluded_vertex_index + 1 ) % 3 ];
				paired_triangles[ 0 ].vertices[ 2 ] = intersection_vertices[ ( secluded_vertex_index + 2 ) % 3 ];

				paired_triangles[ 1 ].vertices[ 0 ] = src_vertices[ ( secluded_vertex_index + 2 ) % 3 ];
				paired_triangles[ 1 ].vertices[ 1 ] = intersection_vertices[ ( secluded_vertex_index + 1 ) % 3 ];
				paired_triangles[ 1 ].vertices[ 2 ] = paired_triangles[ 0 ].vertices[ 0 ];

				// Is secluded vertex above plane? (positive)
				if( ( Vector3( src_vertices[ secluded_vertex_index ].position ) - ( plane.normal * plane.displacement ) ).DotProduct( plane.normal ) >= 0.0f )
				{
					Face secluded_face;
					secluded_face.indices[ 0 ] = geometry_positive.AddVertex( secluded_triangle.vertices[ 0 ] );
					secluded_face.indices[ 1 ] = geometry_positive.AddVertex( secluded_triangle.vertices[ 1 ] );
					secluded_face.indices[ 2 ] = geometry_positive.AddVertex( secluded_triangle.vertices[ 2 ] );

					geometry_positive.AddFace( secluded_face );

					for( size_t i = 0; i < 2; ++i )
					{
						Face pair_face;
						pair_face.indices[ 0 ] = geometry_negative.AddVertex( paired_triangles[ i ].vertices[ 0 ] );
						pair_face.indices[ 1 ] = geometry_negative.AddVertex( paired_triangles[ i ].vertices[ 1 ] );
						pair_face.indices[ 2 ] = geometry_negative.AddVertex( paired_triangles[ i ].vertices[ 2 ] );

						geometry_negative.AddFace( pair_face );
					}
				}
				else
				{
					Face secluded_face;
					secluded_face.indices[ 0 ] = geometry_negative.AddVertex( secluded_triangle.vertices[ 0 ] );
					secluded_face.indices[ 1 ] = geometry_negative.AddVertex( secluded_triangle.vertices[ 1 ] );
					secluded_face.indices[ 2 ] = geometry_negative.AddVertex( secluded_triangle.vertices[ 2 ] );

					geometry_negative.AddFace( secluded_face );

					for( size_t i = 0; i < 2; ++i )
					{
						Face pair_face;
						pair_face.indices[ 0 ] = geometry_positive.AddVertex( paired_triangles[ i ].vertices[ 0 ] );
						pair_face.indices[ 1 ] = geometry_positive.AddVertex( paired_triangles[ i ].vertices[ 1 ] );
						pair_face.indices[ 2 ] = geometry_positive.AddVertex( paired_triangles[ i ].vertices[ 2 ] );

						geometry_positive.AddFace( pair_face );
					}
				}
			}
			else
			{
				// No intersection
				const Vector3 plane_center = plane.Center();
				const float   dot_sum      = plane.normal.DotProduct( Vector3( src_vertices[ 0 ].position ) - plane_center ) +
				                             plane.normal.DotProduct( Vector3( src_vertices[ 1 ].position ) - plane_center ) +
				                             plane.normal.DotProduct( Vector3( src_vertices[ 2 ].position ) - plane_center );

				// Which side of the plane is the triangle on?
				if( std::signbit( dot_sum ) )
				{
					Face secluded_face;
					secluded_face.indices[ 0 ] = geometry_negative.AddVertex( src_vertices[ 0 ] );
					secluded_face.indices[ 1 ] = geometry_negative.AddVertex( src_vertices[ 1 ] );
					secluded_face.indices[ 2 ] = geometry_negative.AddVertex( src_vertices[ 2 ] );

					geometry_negative.AddFace( secluded_face );
				}
				else
				{
					Face secluded_face;
					secluded_face.indices[ 0 ] = geometry_positive.AddVertex( src_vertices[ 0 ] );
					secluded_face.indices[ 1 ] = geometry_positive.AddVertex( src_vertices[ 1 ] );
					secluded_face.indices[ 2 ] = geometry_positive.AddVertex( src_vertices[ 2 ] );

					geometry_positive.AddFace( secluded_face );
				}
			}
		}

//////////////////////////////////////////////////////////////////////////

		std::vector< LineSegment > sorted_seams;
		sorted_seams.push_back( all_seams.front() );
		all_seams.erase( all_seams.begin() );

		DebugManager::GetInstance().PushLineSegment( LineSegment( sorted_seams.front().start + plane.normal * 0.1f, sorted_seams.front().end + plane.normal * 0.1f ), Color( 0.0f, 1.0f, 0.0f ), debug_duration );

		auto find_adjoined_seam = [ & ]( Vector3 point )
		{
			for( auto it = all_seams.begin(); it != all_seams.end(); ++it )
			{
				if( ( it->start - point ).IsZero( 0.00001f ) )
				{
					DebugManager::GetInstance().PushLineSegment( LineSegment( it->start + plane.normal * 0.1f, it->end + plane.normal * 0.1f ), Color( 0.0f, 1.0f, 0.0f ), debug_duration );

					sorted_seams.push_back( *it );

					return it;
				}
				else if( ( it->end - point ).IsZero( 0.00001f ) )
				{
					DebugManager::GetInstance().PushLineSegment( LineSegment( it->end + plane.normal * 0.1f, it->start + plane.normal * 0.1f ), Color( 0.0f, 1.0f, 0.0f ), debug_duration );

					sorted_seams.emplace_back( it->end, it->start );

					return it;
				}
			}

			return all_seams.end();
		};

		auto find_nearest_seam = [ & ]( Vector3 point )
		{
			auto        nearest_seam             = all_seams.end();
			float       nearest_squared_distance = std::numeric_limits< float >::max();
			LineSegment seam_to_be_added;

			for( auto it = all_seams.begin(); it != all_seams.end(); ++it )
			{
				const float squared_distance_to_start = ( it->start - point ).DotProduct();
				const float squared_distance_to_end   = ( it->end - point ).DotProduct();

				if( squared_distance_to_start < nearest_squared_distance || squared_distance_to_end < nearest_squared_distance )
				{
					nearest_seam = it;

					if( squared_distance_to_start < squared_distance_to_end )
					{
						seam_to_be_added         = LineSegment( it->start, it->end );
						nearest_squared_distance = squared_distance_to_start;
					}
					else
					{
						seam_to_be_added         = LineSegment( it->end, it->start );
						nearest_squared_distance = squared_distance_to_end;
					}
				}
			}

			if( nearest_seam != all_seams.end() )
			{
				DebugManager::GetInstance().PushLineSegment( LineSegment( point                  + plane.normal * 0.1f, seam_to_be_added.start + plane.normal * 0.1f ), Color( 1.0f, 0.0f, 1.0f ), debug_duration );
				DebugManager::GetInstance().PushLineSegment( LineSegment( seam_to_be_added.start + plane.normal * 0.1f, seam_to_be_added.end   + plane.normal * 0.1f ), Color( 1.0f, 0.0f, 1.0f ), debug_duration );

				sorted_seams.emplace_back( point, seam_to_be_added.start );
				sorted_seams.emplace_back( std::move( seam_to_be_added ) );
			}

			return nearest_seam;
		};

		while( !all_seams.empty() )
		{
			const LineSegment& primary_seam = sorted_seams.back();

			// Find adjoined seam
			if( auto it = find_adjoined_seam( primary_seam.end ); it != all_seams.end() )
				all_seams.erase( it );
			else if( auto it2 = find_nearest_seam( primary_seam.end ); it2 != all_seams.end() )
				all_seams.erase( it2 );
			else
				break;
		}

//////////////////////////////////////////////////////////////////////////

		const LineSegment&        hull_seam_a                    = sorted_seams[ ( 0 * sorted_seams.size() ) / 3 ];
		const LineSegment&        hull_seam_b                    = sorted_seams[ ( 1 * sorted_seams.size() ) / 3 ];
		const LineSegment&        hull_seam_c                    = sorted_seams[ ( 2 * sorted_seams.size() ) / 3 ];
		const Triangle3D          hull_triangle                  = Triangle3D( hull_seam_a.Center(), hull_seam_b.Center(), hull_seam_c.Center() );
		const bool                hull_is_clockwise_around_plane = hull_triangle.IsClockwiseAround( plane.normal );
		std::vector< Triangle3D > triangles;

		// #FIXME: 2 is temporary
		while( sorted_seams.size() > 2 )
		{
			for( size_t i = 1; i < sorted_seams.size(); ++i )
			{
				auto       a                 = sorted_seams.begin() + i - 1;
				auto       b                 = sorted_seams.begin() + i;
				Triangle3D triangle          = Triangle3D( a->start, a->end, b->end );
				bool       can_make_triangle = triangle.IsClockwiseAround( plane.normal ) == hull_is_clockwise_around_plane;

				if( can_make_triangle )
				{
					triangles.emplace_back( std::move( triangle ) );
					sorted_seams.insert( b + 1, LineSegment( a->start, b->end ) );
					sorted_seams.erase( a, b + 1 );
				}
			}
		}

//////////////////////////////////////////////////////////////////////////

		for( const Triangle3D& triangle : triangles )
		{
			// Positive face
			{
				Face face;
				face.indices[ 0 ] = geometry_positive.AddVertex( Vertex{ Vector4( triangle.a_, 1.0f ) } );
				face.indices[ 1 ] = geometry_positive.AddVertex( Vertex{ Vector4( triangle.b_, 1.0f ) } );
				face.indices[ 2 ] = geometry_positive.AddVertex( Vertex{ Vector4( triangle.c_, 1.0f ) } );

				size_t face_index = geometry_positive.AddFace( face );
				geometry_positive.FlipFaceTowards( face_index, -plane.normal );
			}

			// Negative face
			{
				Face face;
				face.indices[ 0 ] = geometry_negative.AddVertex( Vertex{ Vector4( triangle.a_, 1.0f ) } );
				face.indices[ 1 ] = geometry_negative.AddVertex( Vertex{ Vector4( triangle.c_, 1.0f ) } );
				face.indices[ 2 ] = geometry_negative.AddVertex( Vertex{ Vector4( triangle.b_, 1.0f ) } );

				size_t face_index = geometry_negative.AddFace( face );
				geometry_negative.FlipFaceTowards( face_index, plane.normal );
			}

			DebugManager::GetInstance().PushLineSegment( LineSegment( triangle.a_, triangle.b_ ), Color( 1.0f, 0.0f, 0.0f ), debug_duration );
			DebugManager::GetInstance().PushLineSegment( LineSegment( triangle.b_, triangle.c_ ), Color( 1.0f, 0.0f, 0.0f ), debug_duration );
			DebugManager::GetInstance().PushLineSegment( LineSegment( triangle.c_, triangle.a_ ), Color( 1.0f, 0.0f, 0.0f ), debug_duration );
		}

//////////////////////////////////////////////////////////////////////////

		if( geometry_positive.GetFaceCount() )
		{
			Mesh mesh_positive = geometry_positive.ToMesh( name_ + " (splice, positive)" );
			meshes.emplace_back( std::move( mesh_positive ) );
		}

		if( geometry_negative.GetFaceCount() )
		{
			Mesh mesh_negative = geometry_negative.ToMesh( name_ + " (splice, negative)" );
			meshes.emplace_back( std::move( mesh_negative ) );
		}
	}

	return meshes;
}

ORB_NAMESPACE_END
