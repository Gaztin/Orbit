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
#include "Orbit/Graphics/Model/Mesh.h"
#include "Orbit/Graphics/Shader/VertexLayout.h"
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
	auto         vertex_data   = vertex_count ? std::unique_ptr< uint8_t[]  >( new uint8_t[ vertex_count * vertex_stride ] ) : nullptr;
	auto         index_data    = face_count   ? std::unique_ptr< uint16_t[] >( new uint16_t[ face_count * 3 ] )              : nullptr;

	switch( shape.GetType() )
	{
		case ShapeType::Cube:   { GenerateCubeData(   vertex_data.get(), index_data.get(), vertex_layout ); } break;
		case ShapeType::Sphere: { GenerateSphereData( vertex_data.get(), index_data.get(), vertex_layout ); } break;
	}

	GenerateNormals( vertex_data.get(), index_data.get(), face_count, vertex_layout );

	Mesh mesh;
	mesh.vertex_buffer = std::make_unique< VertexBuffer >( vertex_data.get(), ( selector_vertex_count[ shape.GetType() ] ), vertex_layout.GetStride() );
	mesh.index_buffer  = std::make_unique< IndexBuffer  >( IndexFormat::Word, index_data.get(), ( selector_face_count[ shape.GetType() ] * 3 ) );

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

void MeshFactory::GenerateCubeData( uint8_t* vertex_data, uint16_t* index_data, const VertexLayout& vertex_layout ) const
{
	const size_t vertex_stride = vertex_layout.GetStride();

	for( uint16_t side = 0; side < 6; ++side )
	{
		for( uint16_t point = 0; point < 3; ++point )
			index_data[ side * 6 + point ] = ( side * 4 + point );

		index_data[ side * 6 + 3 ] = ( side * 4 );

		for( uint16_t point = 0; point < 2; ++point )
			index_data[ side * 6 + 4 + point ] = ( side * 4 + 2 + point );
	}

	if( vertex_layout.Contains( VertexComponent::Position ) )
	{
		const size_t                        offset = vertex_layout.OffsetOf( VertexComponent::Position );
		constexpr std::array< uint8_t, 24 > position_indices
		{
			0, 2, 3, 1,
			1, 3, 7, 5,
			5, 7, 6, 4,
			4, 6, 2, 0,
			0, 1, 5, 4,
			7, 3, 2, 6,
		};

		for( size_t i = 0; i < position_indices.size(); ++i )
		{
			Vector4* w = reinterpret_cast< Vector4* >( &vertex_data[ vertex_stride * i + offset ] );
			w->x       = -1.0f + ( 2.0f * ( ( position_indices[ i ] & 1 ) != 0 ) );
			w->y       = -1.0f + ( 2.0f * ( ( position_indices[ i ] & 2 ) != 0 ) );
			w->z       = -1.0f + ( 2.0f * ( ( position_indices[ i ] & 4 ) != 0 ) );
			w->w       =  1.0f;
		}
	}

	if( vertex_layout.Contains( VertexComponent::Normal ) )
	{
		const size_t                   offset = vertex_layout.OffsetOf( VertexComponent::Normal );
		const std::array< Vector3, 6 > normals
		{
			Vector3(  0.0f,  0.0f, -1.0f ),
			Vector3(  1.0f,  0.0f,  0.0f ),
			Vector3(  0.0f,  0.0f,  1.0f ),
			Vector3( -1.0f,  0.0f,  0.0f ),
			Vector3(  0.0f, -1.0f,  0.0f ),
			Vector3(  0.0f,  1.0f,  0.0f ),
		};

		for( size_t side = 0; side < 6; ++side )
		{
			for( size_t corner = 0; corner < 4; ++corner )
			{
				const size_t index = ( side * 4 + corner );
				Vector3*     w     = reinterpret_cast< Vector3* >( &vertex_data[ vertex_stride * index + offset ] );
				*w                 = normals[ side ];
			}
		}
	}

	if( vertex_layout.Contains( VertexComponent::Color ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::Color );

		for( size_t i = 0; i < 24; ++i )
		{
			Color* w = reinterpret_cast< Color* >( &vertex_data[ vertex_stride * i + offset ] );
			w->r     = 0.75f;
			w->g     = 0.75f;
			w->b     = 0.75f;
			w->a     = 1.00f;
		}
	}

	if( vertex_layout.Contains( VertexComponent::TexCoord ) )
	{
		const size_t                   offset = vertex_layout.OffsetOf( VertexComponent::TexCoord );
		const std::array< Vector2, 4 > tex_coords
		{
			Vector2( 0.0f, 1.0f ),
			Vector2( 0.0f, 0.0f ),
			Vector2( 1.0f, 0.0f ),
			Vector2( 1.0f, 1.0f ),
		};

		for( size_t side = 0; side < 6; ++side )
		{
			for( size_t corner = 0; corner < 4; ++corner )
			{
				const size_t index = ( side * 4 + corner );
				Vector2*     w     = reinterpret_cast< Vector2* >( &vertex_data[ vertex_stride * index + offset ] );
				*w                 = tex_coords[ corner ];
			}
		}
	}
}

void MeshFactory::GenerateSphereData( uint8_t* vertex_data, uint16_t* index_data, const VertexLayout& vertex_layout ) const
{
	const size_t vertex_stride = vertex_layout.GetStride();
	size_t       i             = 0;

	// 5 faces around high point
	index_data[ i++ ] = 0;
	index_data[ i++ ] = 11;
	index_data[ i++ ] = 5;
	index_data[ i++ ] = 0;
	index_data[ i++ ] = 5;
	index_data[ i++ ] = 1;
	index_data[ i++ ] = 0;
	index_data[ i++ ] = 1;
	index_data[ i++ ] = 7;
	index_data[ i++ ] = 0;
	index_data[ i++ ] = 7;
	index_data[ i++ ] = 10;
	index_data[ i++ ] = 0;
	index_data[ i++ ] = 10;
	index_data[ i++ ] = 11;

	// 5 adjacent faces 
	index_data[ i++ ] = 1;
	index_data[ i++ ] = 5;
	index_data[ i++ ] = 9;
	index_data[ i++ ] = 5;
	index_data[ i++ ] = 11;
	index_data[ i++ ] = 4;
	index_data[ i++ ] = 11;
	index_data[ i++ ] = 10;
	index_data[ i++ ] = 2;
	index_data[ i++ ] = 10;
	index_data[ i++ ] = 7;
	index_data[ i++ ] = 6;
	index_data[ i++ ] = 7;
	index_data[ i++ ] = 1;
	index_data[ i++ ] = 8;

	// 5 faces around low point
	index_data[ i++ ] = 3;
	index_data[ i++ ] = 9;
	index_data[ i++ ] = 4;
	index_data[ i++ ] = 3;
	index_data[ i++ ] = 4;
	index_data[ i++ ] = 2;
	index_data[ i++ ] = 3;
	index_data[ i++ ] = 2;
	index_data[ i++ ] = 6;
	index_data[ i++ ] = 3;
	index_data[ i++ ] = 6;
	index_data[ i++ ] = 8;
	index_data[ i++ ] = 3;
	index_data[ i++ ] = 8;
	index_data[ i++ ] = 9;

	// 5 adjacent faces
	index_data[ i++ ] = 4;
	index_data[ i++ ] = 9;
	index_data[ i++ ] = 5;
	index_data[ i++ ] = 2;
	index_data[ i++ ] = 4;
	index_data[ i++ ] = 11;
	index_data[ i++ ] = 6;
	index_data[ i++ ] = 2;
	index_data[ i++ ] = 10;
	index_data[ i++ ] = 8;
	index_data[ i++ ] = 6;
	index_data[ i++ ] = 7;
	index_data[ i++ ] = 9;
	index_data[ i++ ] = 8;
	index_data[ i++ ] = 1;

	if( vertex_layout.Contains( VertexComponent::Position ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::Position );
		const float  magic  = 1.0f + ( std::sqrtf( 5.0f ) / 2.0f );

		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 0  ) + offset ] ) = Vector4( -1.0f,  magic, 0.0f, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 1  ) + offset ] ) = Vector4(  1.0f,  magic, 0.0f, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 2  ) + offset ] ) = Vector4( -1.0f, -magic, 0.0f, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 3  ) + offset ] ) = Vector4(  1.0f, -magic, 0.0f, 1.0f );

		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 4  ) + offset ] ) = Vector4(  0.0f, -1.0f,  magic, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 5  ) + offset ] ) = Vector4(  0.0f,  1.0f,  magic, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 6  ) + offset ] ) = Vector4(  0.0f, -1.0f, -magic, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 7  ) + offset ] ) = Vector4(  0.0f,  1.0f, -magic, 1.0f );

		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 8  ) + offset ] ) = Vector4(  magic, 0.0f, -1.0f, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 9  ) + offset ] ) = Vector4(  magic, 0.0f,  1.0f, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 10 ) + offset ] ) = Vector4( -magic, 0.0f, -1.0f, 1.0f );
		reinterpret_cast< Vector4& >( vertex_data[ ( vertex_stride * 11 ) + offset ] ) = Vector4( -magic, 0.0f,  1.0f, 1.0f );
	}

	if( vertex_layout.Contains( VertexComponent::Normal ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::Normal );

		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 0  ) + offset ] ) = Vector3( 0.0f, 0.0f, 1.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 1  ) + offset ] ) = Vector3( 0.0f, 0.0f, 1.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 2  ) + offset ] ) = Vector3( 0.0f, 0.0f, 1.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 3  ) + offset ] ) = Vector3( 0.0f, 0.0f, 1.0f );

		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 4  ) + offset ] ) = Vector3( 1.0f, 0.0f, 0.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 5  ) + offset ] ) = Vector3( 1.0f, 0.0f, 0.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 6  ) + offset ] ) = Vector3( 1.0f, 0.0f, 0.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 7  ) + offset ] ) = Vector3( 1.0f, 0.0f, 0.0f );

		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 8  ) + offset ] ) = Vector3( 0.0f, 1.0f, 0.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 9  ) + offset ] ) = Vector3( 0.0f, 1.0f, 0.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 10 ) + offset ] ) = Vector3( 0.0f, 1.0f, 0.0f );
		reinterpret_cast< Vector3& >( vertex_data[ ( vertex_stride * 11 ) + offset ] ) = Vector3( 0.0f, 1.0f, 0.0f );
	}

	if( vertex_layout.Contains( VertexComponent::TexCoord ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::TexCoord );

		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 0  ) + offset ] ) = Vector2( 0.0f, 1.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 1  ) + offset ] ) = Vector2( 1.0f, 1.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 2  ) + offset ] ) = Vector2( 0.0f, 0.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 3  ) + offset ] ) = Vector2( 1.0f, 0.0f );

		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 4  ) + offset ] ) = Vector2( 0.0f, 1.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 5  ) + offset ] ) = Vector2( 1.0f, 1.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 6  ) + offset ] ) = Vector2( 0.0f, 0.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 7  ) + offset ] ) = Vector2( 1.0f, 0.0f );

		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 8  ) + offset ] ) = Vector2( 0.0f, 1.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 9  ) + offset ] ) = Vector2( 1.0f, 1.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 10 ) + offset ] ) = Vector2( 0.0f, 0.0f );
		reinterpret_cast< Vector2& >( vertex_data[ ( vertex_stride * 11 ) + offset ] ) = Vector2( 1.0f, 0.0f );
	}
}

void MeshFactory::GenerateNormals( uint8_t* vertex_data, const uint16_t* index_data, size_t face_count, const VertexLayout& vertex_layout ) const
{
	if( !vertex_layout.Contains( VertexComponent::Position ) || !vertex_layout.Contains( VertexComponent::Normal ) )
		return;

	const size_t stride        = vertex_layout.GetStride();
	const size_t pos_offset    = vertex_layout.OffsetOf( VertexComponent::Position );
	const size_t normal_offset = vertex_layout.OffsetOf( VertexComponent::Normal );

	for( uint32_t face = 0; face < face_count; ++face )
	{
		const uint16_t triangle_indices[ 3 ]
		{
			index_data[ face * 3 + 0 ],
			index_data[ face * 3 + 1 ],
			index_data[ face * 3 + 2 ],
		};

		const Orbit::Vector3* triangle_positions[ 3 ]
		{
			reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * triangle_indices[ 0 ] + pos_offset ] ),
			reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * triangle_indices[ 1 ] + pos_offset ] ),
			reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * triangle_indices[ 2 ] + pos_offset ] ),
		};

		const Orbit::Vector3 pos0_to_pos1 = ( *triangle_positions[ 1 ] - *triangle_positions[ 0 ] );
		const Orbit::Vector3 pos0_to_pos2 = ( *triangle_positions[ 2 ] - *triangle_positions[ 0 ] );
		const Orbit::Vector3 normal       = ( pos0_to_pos1.CrossProduct( pos0_to_pos2 ) ).Normalized();

		for( size_t triangle_index : triangle_indices )
		{
			Vector3* w = reinterpret_cast< Vector3* >( &vertex_data[ stride * triangle_index + normal_offset ] );
			*w = normal;
		}
	}
}

ORB_NAMESPACE_END
