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

#include <cassert>

ORB_NAMESPACE_BEGIN

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
				geometry.SetFromData( { static_cast< const uint8_t* >( temp_vb_mapped.pData ), vertex_buffer_->GetSize() }, { static_cast< const uint8_t* >( temp_ib_mapped.pData ), index_buffer_->GetSize() }, index_buffer_->GetFormat() );

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

		const size_t   vertex_stride     = vertex_layout_.GetStride();
		const size_t   pos_offset        = vertex_layout_.OffsetOf( VertexComponent::Position );
		const size_t   normal_offset     = vertex_layout_.OffsetOf( VertexComponent::Normal );
		const size_t   color_offset      = vertex_layout_.OffsetOf( VertexComponent::Color );
		const size_t   texcoord_offset   = vertex_layout_.OffsetOf( VertexComponent::TexCoord );
		const Geometry geometry          = ToGeometry();
		Geometry       geometry_positive( vertex_layout_ );
		Geometry       geometry_negative( vertex_layout_ );

		for( auto face : geometry.GetFaces() )
		{
			const Vertex src_vertices[ 3 ]
			{
				geometry.GetVertex( face.indices[ 0 ] ),
				geometry.GetVertex( face.indices[ 1 ] ),
				geometry.GetVertex( face.indices[ 2 ] ),
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

//////////////////////////////////////////////////////////////////////////

		if( geometry_positive.GetFaceCount() )
		{
			Mesh mesh_positive      = geometry_positive.ToMesh( name_ + " (splice, positive)" );
			mesh_positive.transform = transform;
			meshes.emplace_back( std::move( mesh_positive ) );
		}

		if( geometry_negative.GetFaceCount() )
		{
			Mesh mesh_negative      = geometry_negative.ToMesh( name_ + " (splice, negative)" );
			mesh_negative.transform = transform;
			meshes.emplace_back( std::move( mesh_negative ) );
		}
	}

	return meshes;
}

ORB_NAMESPACE_END
