/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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

#include "Model.h"

#include <cstdio>
#include <vector>

#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Math/Vector3.h"

ORB_NAMESPACE_BEGIN

Model::Model( ByteSpan data, const VertexLayout& layout )
{
	/* Assume obj for now */
	ParseOBJ( data, layout );
}

RenderCommand Model::MakeRenderCommand( void )
{
	RenderCommand command;
	command.vertex_buffer = m_vertex_buffer.get();
	command.index_buffer  = m_index_buffer.get();

	return command;
}

void Model::ParseOBJ( ByteSpan data, const VertexLayout& layout )
{
	const char* begin = reinterpret_cast< const char* >( data.begin() );
	const char* end   = reinterpret_cast< const char* >( data.end() );
	const char* it    = begin;

	uint32_t vertex_count = 0;
	uint32_t face_count   = 0;

	/* Count the total number of vertices and faces */
	while( it < end )
	{
		/**/ if( it[ 0 ] == 'v' ) { ++vertex_count; }
		else if( it[ 0 ] == 'f' ) { ++face_count; }

		/* Seek to next line */
		while( it < end && *( it++ ) != '\n' );
	}

	it = begin;

	// #TODO: Calculate index size from @face_count
	uint32_t index_size     = 2;
	uint32_t stride         = 0;
	uint32_t pos_index      = 0;
	uint32_t normal_index   = 0;
	uint32_t color_index    = 0;
	uint32_t texcoord_index = 0;

	for( VertexComponent vc : layout )
	{
		switch( vc )
		{
			case VertexComponent::Position: { pos_index      = stride; stride += sizeof( float ) * 4; } break;
			case VertexComponent::Normal:   { normal_index   = stride; stride += sizeof( float ) * 3; } break;
			case VertexComponent::Color:    { color_index    = stride; stride += sizeof( float ) * 4; } break;
			case VertexComponent::TexCoord: { texcoord_index = stride; stride += sizeof( float ) * 2; } break;
			default:                        {                                                         } break;
		}
	}

	auto     vertex_data   = std::unique_ptr< uint8_t[] >( new uint8_t[ stride * vertex_count ] );
	auto     index_data    = std::unique_ptr< uint16_t[] >( new uint16_t[ index_size * face_count * 3 ] );
	uint32_t vertices_read = 0;
	uint32_t faces_read    = 0;

	for( size_t i = 0; i < vertex_count; ++i )
	{
		float* pos_write      = reinterpret_cast< float* >( &vertex_data[ stride * i + pos_index ] );
		float* color_write    = reinterpret_cast< float* >( &vertex_data[ stride * i + color_index ] );
		float* texcoord_write = reinterpret_cast< float* >( &vertex_data[ stride * i + texcoord_index ] );

		pos_write[ 0 ] = 0.0f;
		pos_write[ 1 ] = 0.0f;
		pos_write[ 2 ] = 0.0f;
		pos_write[ 3 ] = 1.0f;

		color_write[ 0 ] = 0.0f;
		color_write[ 1 ] = 0.0f;
		color_write[ 2 ] = 0.0f;
		color_write[ 3 ] = 1.0f;

		texcoord_write[ 0 ] = 0.0f;
		texcoord_write[ 1 ] = 0.0f;
	}

	/* Parse the vertex and index data */
	while( it < end )
	{
		int bytes_read;

		/* Vertex position */
		{
			float pos[ 3 ];

			if( std::sscanf( it, "v %f %f %f\n%n", &pos[ 0 ], &pos[ 1 ], &pos[ 2 ], &bytes_read ) == 3 )
			{
				float* pos_write = reinterpret_cast< float* >( &vertex_data[ stride * vertices_read + pos_index ] );
				pos_write[ 0 ] = pos[ 0 ];
				pos_write[ 1 ] = pos[ 1 ];
				pos_write[ 2 ] = pos[ 2 ];
				pos_write[ 3 ] = 1.0f;

				++vertices_read;
				it += bytes_read;

				continue;
			}
		}

		/* Face indices */
		{
			uint32_t indices[ 3 ];

			if( std::sscanf( it, "f %u %u %u\n%n", &indices[ 0 ], &indices[ 1 ], &indices[ 2 ], &bytes_read ) == 3 )
			{
				index_data[ faces_read * 3 + 0 ] = static_cast< uint16_t >( indices[ 0 ] - 1 );
				index_data[ faces_read * 3 + 1 ] = static_cast< uint16_t >( indices[ 1 ] - 1 );
				index_data[ faces_read * 3 + 2 ] = static_cast< uint16_t >( indices[ 2 ] - 1 );

				++faces_read;
				it += bytes_read;

				continue;
			}
		}

		/* Seek to next line */
		while( it < end && *( it++ ) != '\n' );
	}

	/* Generate normals */
	for( uint32_t face = 0; face < face_count; ++face )
	{
		const uint16_t index0 = index_data[ face * 3 + 0 ];
		const uint16_t index1 = index_data[ face * 3 + 1 ];
		const uint16_t index2 = index_data[ face * 3 + 2 ];

		const Orbit::Vector3* pos0 = reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * index0 + pos_index ] );
		const Orbit::Vector3* pos1 = reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * index1 + pos_index ] );
		const Orbit::Vector3* pos2 = reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * index2 + pos_index ] );

		const Orbit::Vector3 pos0_to_pos1 = ( *pos1 - *pos0 );
		const Orbit::Vector3 pos0_to_pos2 = ( *pos2 - *pos0 );

		Orbit::Vector3 normal = pos0_to_pos2.CrossProduct( pos0_to_pos1 );
		normal.Normalize();

		Orbit::Vector3* normal0_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * index0 + normal_index ] );
		Orbit::Vector3* normal1_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * index1 + normal_index ] );
		Orbit::Vector3* normal2_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * index2 + normal_index ] );

		*normal0_write = normal;
		*normal1_write = normal;
		*normal2_write = normal;
	}

	/* Create buffers */
	m_vertex_buffer = std::make_unique< VertexBuffer >( vertex_data.get(), vertex_count, stride );
	m_index_buffer  = std::make_unique< IndexBuffer >( IndexFormat::Word, index_data.get(), face_count * 3 );
}

ORB_NAMESPACE_END
