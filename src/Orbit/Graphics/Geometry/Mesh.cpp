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

#include "Orbit/Graphics/Geometry/Face.h"
#include "Orbit/Graphics/Geometry/GeometryData.h"
#include "Orbit/Math/Geometry/LineSegment.h"
#include "Orbit/Math/Geometry/Plane.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

Mesh::Mesh( std::string_view name )
	: name_( name )
{
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

		const size_t    vertex_stride     = vertex_layout_.GetStride();
		const size_t    pos_offset        = vertex_layout_.OffsetOf( VertexComponent::Position );
		const size_t    normal_offset     = vertex_layout_.OffsetOf( VertexComponent::Normal );
		const size_t    color_offset      = vertex_layout_.OffsetOf( VertexComponent::Color );
		const size_t    texcoord_offset   = vertex_layout_.OffsetOf( VertexComponent::TexCoord );
		const uint8_t*  vsrc              = static_cast< const uint8_t* >( vertex_buffer_->MapRead() );
		const uint16_t* isrc              = static_cast< const uint16_t* >( index_buffer_->MapRead() ); // #TODO: Support different index sizes
		const size_t    triangle_count    = ( index_buffer_->GetCount() / 3 );
		GeometryData    geometry_positive = GeometryData( vertex_layout_ );
		GeometryData    geometry_negative = GeometryData( vertex_layout_ );

		for( size_t triangle_index = 0; triangle_index < triangle_count; ++triangle_index )
		{
			const Face   src_face{ isrc[ triangle_index * 3 + 0 ], isrc[ triangle_index * 3 + 1 ], isrc[ triangle_index * 3 + 2 ] };
			const Vertex src_vertices[ 3 ]
			{
				Vertex{ *reinterpret_cast< const Vector4* >( &vsrc[ vertex_stride * src_face.indices[ 0 ] + pos_offset ] ), *reinterpret_cast< const Vector3* >( &vsrc[ vertex_stride * src_face.indices[ 0 ] + normal_offset ] ), *reinterpret_cast< const Color* >( &vsrc[ vertex_stride * src_face.indices[ 0 ] + color_offset ] ), *reinterpret_cast< const Vector2* >( &vsrc[ vertex_stride * src_face.indices[ 0 ] + texcoord_offset ] ) },
				Vertex{ *reinterpret_cast< const Vector4* >( &vsrc[ vertex_stride * src_face.indices[ 1 ] + pos_offset ] ), *reinterpret_cast< const Vector3* >( &vsrc[ vertex_stride * src_face.indices[ 1 ] + normal_offset ] ), *reinterpret_cast< const Color* >( &vsrc[ vertex_stride * src_face.indices[ 1 ] + color_offset ] ), *reinterpret_cast< const Vector2* >( &vsrc[ vertex_stride * src_face.indices[ 1 ] + texcoord_offset ] ) },
				Vertex{ *reinterpret_cast< const Vector4* >( &vsrc[ vertex_stride * src_face.indices[ 2 ] + pos_offset ] ), *reinterpret_cast< const Vector3* >( &vsrc[ vertex_stride * src_face.indices[ 2 ] + normal_offset ] ), *reinterpret_cast< const Color* >( &vsrc[ vertex_stride * src_face.indices[ 2 ] + color_offset ] ), *reinterpret_cast< const Vector2* >( &vsrc[ vertex_stride * src_face.indices[ 2 ] + texcoord_offset ] ) },
			};

			// Edges are in a specific sequence so that 'edges[x]' is the opposite edge of the corner 'src_verticies[x]'
			const LineSegment edges[ 3 ]
			{
				LineSegment( Vector3( src_vertices[ 1 ].position ), Vector3( src_vertices[ 2 ].position ) ),
				LineSegment( Vector3( src_vertices[ 2 ].position ), Vector3( src_vertices[ 0 ].position ) ),
				LineSegment( Vector3( src_vertices[ 0 ].position ), Vector3( src_vertices[ 1 ].position ) ),
			};

			struct Triangle
			{
				Vertex vertices[ 3 ];
			};

//////////////////////////////////////////////////////////////////////////

			const Plane              inverted_plane = Plane( -plane.normal, -plane.displacement );
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
				else if( auto intersection = inverted_plane.Intersect( edges[ i ] ); intersection.index() == 1 )
				{
					intersections[ i ] = std::move( std::get< 1 >( intersection ) );
					++intersection_count;
				}
				else
				{
					secluded_vertex_index = i;
				}
			}

			if( intersection_count == 2 )
			{
				const std::array< Vertex, 3 > intersection_vertices
				{
					Vertex{ Vector4( intersections[ 0 ], 1.0f ), src_vertices[ secluded_vertex_index ].normal, src_vertices[ secluded_vertex_index ].color, ( src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].tex_coord + ( ( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].tex_coord - src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].tex_coord ) * ( LineSegment( intersections[ 0 ], Vector3( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].position ) ).Length() / LineSegment( Vector3( src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].position ), Vector3( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].position ) ).Length() ) ) ) },
					Vertex{ Vector4( intersections[ 1 ], 1.0f ), src_vertices[ secluded_vertex_index ].normal, src_vertices[ secluded_vertex_index ].color, ( src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].tex_coord + ( ( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].tex_coord - src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].tex_coord ) * ( LineSegment( intersections[ 1 ], Vector3( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].position ) ).Length() / LineSegment( Vector3( src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].position ), Vector3( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].position ) ).Length() ) ) ) },
					Vertex{ Vector4( intersections[ 2 ], 1.0f ), src_vertices[ secluded_vertex_index ].normal, src_vertices[ secluded_vertex_index ].color, ( src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].tex_coord + ( ( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].tex_coord - src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].tex_coord ) * ( LineSegment( intersections[ 2 ], Vector3( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].position ) ).Length() / LineSegment( Vector3( src_vertices[ ( secluded_vertex_index + 1 ) % 3 ].position ), Vector3( src_vertices[ ( secluded_vertex_index + 2 ) % 3 ].position ) ).Length() ) ) ) },
				};

				Triangle secluded_triangle;
				Triangle paired_triangles[ 2 ];

				secluded_triangle.vertices[ 0 ] = src_vertices[ secluded_vertex_index ];
				secluded_triangle.vertices[ 1 ] = intersection_vertices[ ( secluded_vertex_index + 2 ) % 3 ];
				secluded_triangle.vertices[ 2 ] = intersection_vertices[ ( secluded_vertex_index + 1 ) % 3 ];

				paired_triangles[ 0 ].vertices[ 0 ] = src_vertices[ ( secluded_vertex_index + 1 ) % 3 ];
				paired_triangles[ 0 ].vertices[ 1 ] = intersection_vertices[ ( secluded_vertex_index + 1 ) % 3 ];
				paired_triangles[ 0 ].vertices[ 2 ] = intersection_vertices[ ( secluded_vertex_index + 2 ) % 3 ];

				paired_triangles[ 1 ].vertices[ 0 ] = src_vertices[ ( secluded_vertex_index + 2 ) % 3 ];
				paired_triangles[ 1 ].vertices[ 1 ] = intersection_vertices[ ( secluded_vertex_index + 1 ) % 3 ];
				paired_triangles[ 1 ].vertices[ 2 ] = paired_triangles[ 0 ].vertices[ 0 ];

//////////////////////////////////////////////////////////////////////////

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

		index_buffer_->Unmap();
		vertex_buffer_->Unmap();

//////////////////////////////////////////////////////////////////////////

		// Debug purposes
		{
			for( size_t i = 0; i < geometry_positive.GetVertexCount(); ++i )
			{
				Vertex v = geometry_positive.GetVertex( i );
				v.position += Vector4( plane.normal, 0.0f ) * 0.05f;
				geometry_positive.SetVertex( i, v );
			}
			for( size_t i = 0; i < geometry_negative.GetVertexCount(); ++i )
			{
				Vertex v = geometry_negative.GetVertex( i );
				v.position -= Vector4( plane.normal, 0.0f ) * 0.05f;
				geometry_negative.SetVertex( i, v );
			}
		}

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
