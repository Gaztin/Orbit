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

#include <cassert>
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

	/* Count the total number of vertices and faces */
	uint32_t vertex_count = 0;
	uint32_t face_count   = 0;
	while( it < end )
	{
		/**/ if( it[ 0 ] == 'v' ) { ++vertex_count; }
		else if( it[ 0 ] == 'f' ) { ++face_count; }

		/* Seek to next line */
		while( it < end && *( it++ ) != '\n' );
	}

	it = begin;

	uint8_t index_size = 8;
	/**/ if( vertex_count < std::numeric_limits< uint8_t  >::max() ) { index_size = 1; }
	else if( vertex_count < std::numeric_limits< uint16_t >::max() ) { index_size = 2; }
	else if( vertex_count < std::numeric_limits< uint32_t >::max() ) { index_size = 4; }

	uint32_t stride          = layout.GetStride();
	uint32_t pos_offset      = layout.OffsetOf( VertexComponent::Position );
	uint32_t normal_offset   = layout.OffsetOf( VertexComponent::Normal );
	uint32_t color_offset    = layout.OffsetOf( VertexComponent::Color );
	uint32_t texcoord_offset = layout.OffsetOf( VertexComponent::TexCoord );
	auto     vertex_data     = std::unique_ptr< uint8_t[] >( new uint8_t[ stride * vertex_count ] );
	auto     index_data      = std::unique_ptr< uint8_t[] >( new uint8_t[ index_size * face_count * 3 ] );

	/* Clear data before read */
	for( size_t i = 0; i < vertex_count; ++i )
	{
		if( layout.Contains( VertexComponent::Position ) )
		{
			float* pos_write = reinterpret_cast< float* >( &vertex_data[ stride * i + pos_offset ] );
			pos_write[ 0 ] = 0.0f;
			pos_write[ 1 ] = 0.0f;
			pos_write[ 2 ] = 0.0f;
			pos_write[ 3 ] = 1.0f;
		}

		if( layout.Contains( VertexComponent::Normal ) )
		{
			float* normal_write = reinterpret_cast< float* >( &vertex_data[ stride * i + normal_offset ] );
			normal_write[ 0 ] = 0.0f;
			normal_write[ 1 ] = 0.0f;
			normal_write[ 2 ] = 0.0f;
		}

		if( layout.Contains( VertexComponent::Color ) )
		{
			float* color_write = reinterpret_cast< float* >( &vertex_data[ stride * i + color_offset ] );
			color_write[ 0 ] = 0.0f;
			color_write[ 1 ] = 0.0f;
			color_write[ 2 ] = 0.0f;
			color_write[ 3 ] = 1.0f;
		}

		if( layout.Contains( VertexComponent::TexCoord ) )
		{
			float* texcoord_write = reinterpret_cast< float* >( &vertex_data[ stride * i + texcoord_offset ] );
			texcoord_write[ 0 ] = 0.0f;
			texcoord_write[ 1 ] = 0.0f;
		}
	}

	uint32_t vertices_read  = 0;
	uint32_t faces_read     = 0;

	/* Parse the vertex and index data */
	while( it < end )
	{
		int bytes_read;

		/* Vertex position */
		if( layout.Contains( VertexComponent::Position ) )
		{
			float pos[ 3 ];

			if( std::sscanf( it, "v %f %f %f\n%n", &pos[ 0 ], &pos[ 1 ], &pos[ 2 ], &bytes_read ) == 3 )
			{
				float* pos_write = reinterpret_cast< float* >( &vertex_data[ stride * vertices_read + pos_offset ] );
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
			uint64_t indices[ 3 ];

			if( std::sscanf( it, "f %llu %llu %llu\n%n", &indices[ 0 ], &indices[ 1 ], &indices[ 2 ], &bytes_read ) == 3 )
			{
				switch( index_size )
				{
					case 1:
					{
						uint8_t* index_write = &index_data[ index_size * faces_read * 3 ];
						*( index_write + 0 ) = static_cast< uint8_t >( indices[ 0 ] - 1 );
						*( index_write + 1 ) = static_cast< uint8_t >( indices[ 1 ] - 1 );
						*( index_write + 2 ) = static_cast< uint8_t >( indices[ 2 ] - 1 );

					} break;

					case 2:
					{
						uint16_t* index_write = reinterpret_cast< uint16_t* >( &index_data[ index_size * faces_read * 3 ] );
						*( index_write + 0 ) = static_cast< uint16_t >( indices[ 0 ] - 1 );
						*( index_write + 1 ) = static_cast< uint16_t >( indices[ 1 ] - 1 );
						*( index_write + 2 ) = static_cast< uint16_t >( indices[ 2 ] - 1 );
						

					} break;

					case 4:
					{
						uint32_t* index_write = reinterpret_cast< uint32_t* >( &index_data[ index_size * faces_read * 3 ] );
						*( index_write + 0 ) = static_cast< uint32_t >( indices[ 0 ] - 1 );
						*( index_write + 1 ) = static_cast< uint32_t >( indices[ 1 ] - 1 );
						*( index_write + 2 ) = static_cast< uint32_t >( indices[ 2 ] - 1 );

					} break;

					case 8:
					{
						uint64_t* index_write = reinterpret_cast< uint64_t* >( &index_data[ index_size * faces_read * 3 ] );
						*( index_write + 0 ) = static_cast< uint64_t >( indices[ 0 ] - 1 );
						*( index_write + 1 ) = static_cast< uint64_t >( indices[ 1 ] - 1 );
						*( index_write + 2 ) = static_cast< uint64_t >( indices[ 2 ] - 1 );

					} break;
				}

				++faces_read;
				it += bytes_read;

				continue;
			}
		}

		/* Seek to next line */
		while( it < end && *( it++ ) != '\n' );
	}

	/* Generate normals */
	if( layout.Contains( VertexComponent::Normal ) )
	{
		for( uint32_t face = 0; face < face_count; ++face )
		{
			size_t indices[ 3 ];

			switch( index_size )
			{
				case 1:
				{
					const uint8_t* index_read = &index_data[ 1 * face * 3 ];
					indices[ 0 ] = static_cast< size_t >( index_read[ 0 ] );
					indices[ 1 ] = static_cast< size_t >( index_read[ 1 ] );
					indices[ 2 ] = static_cast< size_t >( index_read[ 2 ] );

				} break;

				case 2:
				{
					const uint16_t* index_read = reinterpret_cast< const uint16_t* >( &index_data[ 2 * face * 3 ] );
					indices[ 0 ] = static_cast< size_t >( index_read[ 0 ] );
					indices[ 1 ] = static_cast< size_t >( index_read[ 1 ] );
					indices[ 2 ] = static_cast< size_t >( index_read[ 2 ] );

				} break;

				case 4:
				{
					const uint32_t* index_read = reinterpret_cast< const uint32_t* >( &index_data[ 4 * face * 3 ] );
					indices[ 0 ] = static_cast< size_t >( index_read[ 0 ] );
					indices[ 1 ] = static_cast< size_t >( index_read[ 1 ] );
					indices[ 2 ] = static_cast< size_t >( index_read[ 2 ] );

				} break;

				case 8:
				{
					const uint64_t* index_read = reinterpret_cast< const uint64_t* >( &index_data[ 8 * face * 3 ] );
					indices[ 0 ] = static_cast< size_t >( index_read[ 0 ] );
					indices[ 1 ] = static_cast< size_t >( index_read[ 1 ] );
					indices[ 2 ] = static_cast< size_t >( index_read[ 2 ] );

				} break;

				default:
				{
					assert( false );

				} break;
			}

			const Orbit::Vector3* positions[ 3 ]
			{
				reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * indices[ 0 ] + pos_offset ] ),
				reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * indices[ 1 ] + pos_offset ] ),
				reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * indices[ 2 ] + pos_offset ] ),
			};

			const Orbit::Vector3 pos0_to_pos1 = ( *positions[ 1 ] - *positions[ 0 ] );
			const Orbit::Vector3 pos0_to_pos2 = ( *positions[ 2 ] - *positions[ 0 ] );

			Orbit::Vector3 normal = pos0_to_pos2.CrossProduct( pos0_to_pos1 );
			normal.Normalize();

			Orbit::Vector3* normal0_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * indices[ 0 ] + normal_offset ] );
			Orbit::Vector3* normal1_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * indices[ 1 ] + normal_offset ] );
			Orbit::Vector3* normal2_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * indices[ 2 ] + normal_offset ] );

			*normal0_write = normal;
			*normal1_write = normal;
			*normal2_write = normal;
		}
	}

	/* Create buffers */
	m_vertex_buffer = std::make_unique< VertexBuffer >( vertex_data.get(), vertex_count, stride );
	m_index_buffer  = std::make_unique< IndexBuffer >( IndexFormat::Word, index_data.get(), face_count * 3 );
}

ORB_NAMESPACE_END
