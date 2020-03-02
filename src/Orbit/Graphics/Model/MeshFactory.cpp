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

#include "Orbit/Core/Shape/IShape.h"
#include "Orbit/Core/Utility/Color.h"
#include "Orbit/Core/Utility/Selector.h"
#include "Orbit/Graphics/Model/Mesh.h"
#include "Orbit/Graphics/Shader/VertexLayout.h"
#include "Orbit/Math/Vector2.h"

#include <array>

ORB_NAMESPACE_BEGIN

static const Selector< ShapeType, size_t > selector_vertex_count
{
	{ ShapeType::Cube, 24 },
};

static const Selector< ShapeType, size_t > selector_face_count
{
	{ ShapeType::Cube, 12 },
};

Mesh MeshFactory::CreateMeshFromShape( const IShape& shape, const VertexLayout& vertex_layout ) const
{
	const size_t vertex_stride = vertex_layout.GetStride();
	const size_t vertex_count  = selector_vertex_count[ shape.GetType() ];
	const size_t face_count    = selector_face_count[ shape.GetType() ];
	auto         vertex_data   = std::unique_ptr< uint8_t[] >( new uint8_t[ vertex_count * vertex_stride ] );
	auto         index_data    = std::unique_ptr< uint16_t[] >( new uint16_t[ face_count * 3 ] );

	switch( shape.GetType() )
	{
		case ShapeType::Cube: { FillCubeData( vertex_data.get(), index_data.get(), vertex_layout ); } break;
	}

	Mesh mesh;
	mesh.vertex_buffer = std::make_unique< VertexBuffer >( vertex_data.get(), ( selector_vertex_count[ shape.GetType() ] ), vertex_layout.GetStride() );
	mesh.index_buffer  = std::make_unique< IndexBuffer  >( IndexFormat::Word, index_data.get(), ( selector_face_count[ shape.GetType() ] * 3 ) );

	return mesh;
}

void MeshFactory::FillCubeData( uint8_t* vertex_data, uint16_t* index_data, const VertexLayout& vertex_layout ) const
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
			w->x       = -0.5f + ( 1.0f * ( ( position_indices[ i ] & 1 ) != 0 ) );
			w->y       = -0.5f + ( 1.0f * ( ( position_indices[ i ] & 2 ) != 0 ) );
			w->z       = -0.5f + ( 1.0f * ( ( position_indices[ i ] & 4 ) != 0 ) );
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

ORB_NAMESPACE_END
