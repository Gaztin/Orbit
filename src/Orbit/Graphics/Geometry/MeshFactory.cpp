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

#include "MeshFactory.h"

#include "Orbit/Core/Shape/CubeShape.h"
#include "Orbit/Core/Shape/SphereShape.h"
#include "Orbit/Core/Utility/Color.h"
#include "Orbit/Core/Utility/Selector.h"
#include "Orbit/Graphics/Geometry/GeometryData.h"
#include "Orbit/Graphics/Geometry/Mesh.h"
#include "Orbit/Graphics/Geometry/VertexLayout.h"
#include "Orbit/Math/Vector2.h"

#include <array>

ORB_NAMESPACE_BEGIN

static const Selector< ShapeType, size_t > selector_vertex_count
{
	{ ShapeType::Cube,   24 },
	{ ShapeType::Sphere, 12 },
};

static const Selector< ShapeType, size_t > selector_face_count
{
	{ ShapeType::Cube,   12 },
	{ ShapeType::Sphere, 20 },
};

Mesh MeshFactory::CreateMeshFromShape( const IShape& shape, const VertexLayout& vertex_layout ) const
{
	const size_t vertex_stride = vertex_layout.GetStride();
	const size_t vertex_count  = selector_vertex_count[ shape.GetType() ];
	const size_t face_count    = selector_face_count[ shape.GetType() ];
	GeometryData geometry_data = GeometryData( vertex_count, vertex_layout );

	geometry_data.Reserve( vertex_count, face_count );

	switch( shape.GetType() )
	{
		case ShapeType::Cube:   { GenerateCubeData( geometry_data ); } break;
		case ShapeType::Sphere: { GenerateSphereData( geometry_data ); } break;
	}

	GenerateNormals( geometry_data );

	Mesh mesh = geometry_data.ToMesh();

	switch( shape.GetType() )
	{
		case ShapeType::Cube:
		{
			const CubeShape& cube_shape = static_cast< const CubeShape& >( shape );

			mesh.transform.Scale( Vector3( cube_shape.HalfExtent() ) );

		} break;

		case ShapeType::Sphere:
		{
			const SphereShape& cube_shape = static_cast< const SphereShape& >( shape );

			mesh.transform.Scale( Vector3( cube_shape.Radius() ) );

		} break;
	}

	return mesh;
}

void MeshFactory::GenerateCubeData( GeometryData& geometry_data ) const
{
	for( size_t side = 0; side < 6; ++side )
	{
		geometry_data.AddFace(
			{ ( side * 4 + 0 ),
			  ( side * 4 + 1 ),
			  ( side * 4 + 2 ) } );

		geometry_data.AddFace(
			{ ( side * 4 ),
			  ( side * 4 + 2 ),
			  ( side * 4 + 3 ) } );
	}

	constexpr std::array< uint8_t, 24 > position_indices
	{
		0, 2, 3, 1,
		1, 3, 7, 5,
		5, 7, 6, 4,
		4, 6, 2, 0,
		0, 1, 5, 4,
		7, 3, 2, 6,
	};

	const std::array< Vector3, 6 > normals
	{
		Vector3(  0.0f,  0.0f, -1.0f ),
		Vector3(  1.0f,  0.0f,  0.0f ),
		Vector3(  0.0f,  0.0f,  1.0f ),
		Vector3( -1.0f,  0.0f,  0.0f ),
		Vector3(  0.0f, -1.0f,  0.0f ),
		Vector3(  0.0f,  1.0f,  0.0f ),
	};

	const std::array< Vector2, 4 > tex_coords
	{
		Vector2( 0.0f, 1.0f ),
		Vector2( 0.0f, 0.0f ),
		Vector2( 1.0f, 0.0f ),
		Vector2( 1.0f, 1.0f ),
	};

	for( size_t i = 0; i < position_indices.size(); ++i )
	{
		const Vector4 pos( -1.0f + ( 2.0f * ( ( position_indices[ i ] & 1 ) != 0 ) ),
		                   -1.0f + ( 2.0f * ( ( position_indices[ i ] & 2 ) != 0 ) ),
		                   -1.0f + ( 2.0f * ( ( position_indices[ i ] & 4 ) != 0 ) ),
		                    1.0f );
		const Color color( 0.75f, 0.75f, 0.75f, 1.0f );

		geometry_data.AddVertex( { pos, normals[ i / 4 ], color, tex_coords[ i % 4 ] } );
	}
}

void MeshFactory::GenerateSphereData( GeometryData& geometry_data ) const
{
	using Brim = std::array< uint16_t, 5 >;

	struct Hat
	{
		uint16_t crown;
		Brim     brim;
	};

	const std::array< Hat, 2 > hats
	{
		Hat{ { 0 }, { 11, 5, 1, 7, 10 } },
		Hat{ { 3 }, { 9,  4, 2, 6, 8  } },
	};

	for( size_t h = 0; h < 2; ++h )
	{
		for( size_t b = 0; b < 5; ++b ) geometry_data.AddFace( { hats[ h ].crown,     hats[ h             ].brim[ b                 ], hats[ h ].brim[ ( b + 1 ) % 5 ] } );
		for( size_t b = 0; b < 5; ++b ) geometry_data.AddFace( { hats[ h ].brim[ b ], hats[ ( h + 1 ) % 2 ].brim[ ( 5 + 1 - b ) % 5 ], hats[ h ].brim[ ( b + 1 ) % 5 ] } );
	}

//////////////////////////////////////////////////////////////////////////

	//                         Position                                                 Normal                       Color                               TexCoord
	geometry_data.AddVertex( { Vector4( -1.0f,        GoldenRatio, 0.0f,        1.0f ), Vector3( 0.0f, 0.0f, 1.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 0.0f, 1.0f ) } );
	geometry_data.AddVertex( { Vector4(  1.0f,        GoldenRatio, 0.0f,        1.0f ), Vector3( 0.0f, 0.0f, 1.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 1.0f, 1.0f ) } );
	geometry_data.AddVertex( { Vector4( -1.0f,       -GoldenRatio, 0.0f,        1.0f ), Vector3( 0.0f, 0.0f, 1.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 0.0f, 0.0f ) } );
	geometry_data.AddVertex( { Vector4(  1.0f,       -GoldenRatio, 0.0f,        1.0f ), Vector3( 0.0f, 0.0f, 1.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 1.0f, 0.0f ) } );
	geometry_data.AddVertex( { Vector4(  0.0f,       -1.0f,        GoldenRatio, 1.0f ), Vector3( 1.0f, 0.0f, 0.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 0.0f, 1.0f ) } );
	geometry_data.AddVertex( { Vector4(  0.0f,        1.0f,        GoldenRatio, 1.0f ), Vector3( 1.0f, 0.0f, 0.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 1.0f, 1.0f ) } );
	geometry_data.AddVertex( { Vector4(  0.0f,       -1.0f,       -GoldenRatio, 1.0f ), Vector3( 1.0f, 0.0f, 0.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 0.0f, 0.0f ) } );
	geometry_data.AddVertex( { Vector4(  0.0f,        1.0f,       -GoldenRatio, 1.0f ), Vector3( 1.0f, 0.0f, 0.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 1.0f, 0.0f ) } );
	geometry_data.AddVertex( { Vector4(  GoldenRatio, 0.0f,       -1.0f,        1.0f ), Vector3( 0.0f, 1.0f, 0.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 0.0f, 1.0f ) } );
	geometry_data.AddVertex( { Vector4(  GoldenRatio, 0.0f,        1.0f,        1.0f ), Vector3( 0.0f, 1.0f, 0.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 1.0f, 1.0f ) } );
	geometry_data.AddVertex( { Vector4( -GoldenRatio, 0.0f,       -1.0f,        1.0f ), Vector3( 0.0f, 1.0f, 0.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 0.0f, 0.0f ) } );
	geometry_data.AddVertex( { Vector4( -GoldenRatio, 0.0f,        1.0f,        1.0f ), Vector3( 0.0f, 1.0f, 0.0f ), Color( 0.75f, 0.75f, 0.75f, 1.0f ), Vector2( 1.0f, 0.0f ) } );
}

void MeshFactory::GenerateNormals( GeometryData& geometry_data ) const
{
	for( Face face : geometry_data.GetFaces() )
	{
		const Vertex triangle_vertices[ 3 ]
		{
			geometry_data.GetVertex( face.indices[ 0 ] ),
			geometry_data.GetVertex( face.indices[ 1 ] ),
			geometry_data.GetVertex( face.indices[ 2 ] ),
		};

		const Orbit::Vector3 pos0_to_pos1 = Vector3( triangle_vertices[ 1 ].position - triangle_vertices[ 0 ].position );
		const Orbit::Vector3 pos0_to_pos2 = Vector3( triangle_vertices[ 2 ].position - triangle_vertices[ 0 ].position );
		const Orbit::Vector3 normal       = ( pos0_to_pos1.CrossProduct( pos0_to_pos2 ) ).Normalized();

		for( size_t i = 0; i < 3; ++i )
		{
			Vertex vertex = triangle_vertices[ i ];
			vertex.normal = normal;

			geometry_data.SetVertex( face.indices[ i ], vertex );
		}
	}
}

ORB_NAMESPACE_END
