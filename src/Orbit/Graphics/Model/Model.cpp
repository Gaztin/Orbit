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
#include <sstream>
#include <string>
#include <vector>

#include "Orbit/Core/IO/Parser/XML/XMLParser.h"
#include "Orbit/Core/IO/Log.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Math/Vector3.h"

ORB_NAMESPACE_BEGIN

Model::Model( ByteSpan data, const VertexLayout& layout )
{
	if( !( ParseCollada( data, layout ) ||
	       ParseOBJ( data, layout ) ) )
	{
		LogError( "Failed to load model. Unsupported format." );
	}
}

static void ColladaParseNodeRecursive( const XMLElement& node, const Matrix4& parent_bind_transform, Joint& joint, size_t& next_id )
{
	{
		std::istringstream ss( node[ "matrix" ].content );

		for( size_t i = 0; i < 16; ++i )
			ss >> joint.local_bind_transform[ i ];
	}

	/* Collada matrices are column-major */
	joint.local_bind_transform.Transpose();

	joint.bind_transform = ( parent_bind_transform * joint.local_bind_transform );

	joint.inverse_bind_transform = joint.bind_transform;
	joint.inverse_bind_transform.Invert();

	for( const XMLElement& child : node )
	{
		if( child.name != "node" )
			continue;

		Joint new_joint;
		new_joint.id   = ( next_id++ );
		new_joint.name = child.Attribute( "sid" );

		ColladaParseNodeRecursive( child, joint.bind_transform, new_joint, next_id );

		joint.children.push_back( new_joint );
	}
}

bool Model::ParseCollada( ByteSpan data, const VertexLayout& layout )
{
	const XMLParser xml_parser( data );

	if( !xml_parser.IsGood() )
		return false;

	const XMLElement& collada = xml_parser.GetRootElement()[ "COLLADA" ];

	for( const XMLElement& geometry : collada[ "library_geometries" ] )
	{
		if( geometry.name == "geometry" )
		{
			const std::string geometry_id( geometry.Attribute( "id" ) );

			Mesh mesh;
			mesh.name = geometry.Attribute( "name" );

			const XMLElement& polylist = geometry[ "mesh" ][ "polylist" ];

			size_t vertex_count = 0;
			/* Peek the number of vertices */
			{
				std::string positions_source( geometry[ "mesh" ][ "vertices" ].ChildWithAttribute( "input", "semantic", "POSITION" ).Attribute( "source" ) );
				positions_source.erase( positions_source.begin() );

				const XMLElement&  source = geometry[ "mesh" ].ChildWithAttribute( "source", "id", positions_source );
				std::istringstream ss( std::string( source[ "float_array" ].Attribute( "count" ) ) );
				ss >> vertex_count;
				assert( vertex_count % 3 == 0 );
				vertex_count /= 3;
			}

			size_t face_count = 0;
			/* Peek the number of faces */
			{
				std::istringstream ss( std::string( polylist.Attribute( "count" ) ) );
				ss >> face_count;
			}

			/* Opt out if not COLLADA */
			if( vertex_count == 0 || face_count == 0 )
				return false;

			uint8_t index_size = 4;
			if( vertex_count < std::numeric_limits< uint16_t >::max() ) { index_size = 2; }

			const size_t vertex_stride    = layout.GetStride();
			const size_t pos_offset       = layout.OffsetOf( VertexComponent::Position );
			const size_t normal_offset    = layout.OffsetOf( VertexComponent::Normal );
			const size_t color_offset     = layout.OffsetOf( VertexComponent::Color );
			const size_t texcoord_offset  = layout.OffsetOf( VertexComponent::TexCoord );
			const size_t joint_ids_offset = layout.OffsetOf( VertexComponent::JointIDs );
			const size_t weights_offset   = layout.OffsetOf( VertexComponent::Weights );
			auto         vertex_data      = std::unique_ptr< uint8_t[] >( new uint8_t[ vertex_stride * vertex_count ] );
			auto         index_data       = std::unique_ptr< uint8_t[] >( new uint8_t[ index_size * face_count * 3 ] );

			/* Clear data before read */
			for( size_t i = 0; i < vertex_count; ++i )
			{
				if( layout.Contains( VertexComponent::Position ) )
				{
					float* pos_write = reinterpret_cast< float* >( &vertex_data[ vertex_stride * i + pos_offset ] );
					pos_write[ 0 ] = 0.0f;
					pos_write[ 1 ] = 0.0f;
					pos_write[ 2 ] = 0.0f;
					pos_write[ 3 ] = 1.0f;
				}

				if( layout.Contains( VertexComponent::Normal ) )
				{
					float* normal_write = reinterpret_cast< float* >( &vertex_data[ vertex_stride * i + normal_offset ] );
					normal_write[ 0 ] = 0.0f;
					normal_write[ 1 ] = 0.0f;
					normal_write[ 2 ] = 0.0f;
				}

				if( layout.Contains( VertexComponent::Color ) )
				{
					float* color_write = reinterpret_cast< float* >( &vertex_data[ vertex_stride * i + color_offset ] );
					color_write[ 0 ] = 0.75f;
					color_write[ 1 ] = 0.75f;
					color_write[ 2 ] = 0.75f;
					color_write[ 3 ] = 1.0f;
				}

				if( layout.Contains( VertexComponent::TexCoord ) )
				{
					float* texcoord_write = reinterpret_cast< float* >( &vertex_data[ vertex_stride * i + texcoord_offset ] );
					texcoord_write[ 0 ] = 0.0f;
					texcoord_write[ 1 ] = 0.0f;
				}

				if( layout.Contains( VertexComponent::JointIDs ) )
				{
					int* joint_ids_write = reinterpret_cast< int* >( &vertex_data[ vertex_stride * i + joint_ids_offset ] );
					joint_ids_write[ 0 ] = 0;
					joint_ids_write[ 1 ] = 0;
					joint_ids_write[ 2 ] = 0;
					joint_ids_write[ 3 ] = 0;
				}

				if( layout.Contains( VertexComponent::Weights ) )
				{
					float* weights_write = reinterpret_cast< float* >( &vertex_data[ vertex_stride * i + weights_offset ] );
					weights_write[ 0 ] = 1.0f;
					weights_write[ 1 ] = 0.0f;
					weights_write[ 2 ] = 0.0f;
					weights_write[ 3 ] = 0.0f;
				}
			}

			if( layout.Contains( VertexComponent::Position ) )
			{
				std::string source_id( geometry[ "mesh" ][ "vertices" ].ChildWithAttribute( "input", "semantic", "POSITION" ).Attribute( "source" ) );
				source_id.erase( source_id.begin() );

				const XMLElement& source = geometry[ "mesh" ].ChildWithAttribute( "source", "id", source_id );

				size_t stride = 0;
				{
					std::istringstream ss( std::string( source[ "technique_common" ][ "accessor" ].Attribute( "stride" ) ) );
					ss >> stride;
				}

				std::istringstream ss( source[ "float_array" ].content );

				for( size_t i = 0; i < vertex_count; ++i )
				{
					float* pos_write = reinterpret_cast< float* >( &vertex_data[ i * vertex_stride + pos_offset ] );

					for( size_t c = 0; c < stride; ++c )
						ss >> pos_write[ c ];
				}
			}

			if( layout.Contains( VertexComponent::Normal ) )
			{
				std::string source_id( polylist.ChildWithAttribute( "input", "semantic", "NORMAL" ).Attribute( "source" ) );
				source_id.erase( source_id.begin() );

				const XMLElement& source = geometry[ "mesh" ].ChildWithAttribute( "source", "id", source_id );

				size_t stride = 0;
				{
					std::istringstream ss( std::string( source[ "technique_common" ][ "accessor" ].Attribute( "stride" ) ) );
					ss >> stride;
				}

				std::istringstream ss( source[ "float_array" ].content );

				for( size_t i = 0; i < vertex_count; ++i )
				{
					float* normal_write = reinterpret_cast< float* >( &vertex_data[ i * vertex_stride + normal_offset ] );

					for( size_t c = 0; c < stride; ++c )
						ss >> normal_write[ c ];
				}
			}

			/* Write to index data */
			{
				size_t input_count = 0;
				for( const XMLElement& input : geometry[ "mesh" ][ "polylist" ] )
				{
					if( input.name == "input" )
						++input_count;
				}

				std::istringstream ss( geometry[ "mesh" ][ "polylist" ][ "p" ].content );

				for( size_t i = 0; i < face_count * 3; ++i )
				{
					switch( index_size )
					{
						case 1:
						{
							uint8_t* index_write = &index_data[ i ];
							ss >> *index_write;
						} break;

						case 2:
						{
							uint16_t* index_write = reinterpret_cast< uint16_t* >( &index_data[ i * 2 ] );
							ss >> *index_write;
						} break;

						case 4:
						{
							uint32_t* index_write = reinterpret_cast< uint32_t* >( &index_data[ i * 4 ] );
							ss >> *index_write;
						} break;
					}

					for( size_t input = 1; input < input_count; ++input )
					{
						size_t dummy;
						ss >> dummy;
					}
				}
			}

			/* Find controller data in other library */
			for( const XMLElement& controller : collada[ "library_controllers" ] )
			{
				const std::string controller_id( controller.Attribute( "id" ) );
				const XMLElement& skin = controller[ "skin" ];
				std::string       skin_source_id( skin.Attribute( "source" ) );
				skin_source_id.erase( skin_source_id.begin() );

				if( skin_source_id == geometry_id )
				{
					const XMLElement& vertex_weights = skin[ "vertex_weights" ];
					std::string       weight_source_id( vertex_weights.ChildWithAttribute( "input", "semantic", "WEIGHT" ).Attribute( "source" ) );
					weight_source_id.erase( weight_source_id.begin() );

					std::vector< float > weights;
					for( const XMLElement& source : skin )
					{
						const std::string source_id( source.Attribute( "id" ) );

						if( weight_source_id == source_id )
						{
							const XMLElement& float_array = source[ "float_array" ];

							size_t count = 0;
							{
								std::istringstream ss( std::string( float_array.Attribute( "count" ) ) );
								ss >> count;
							}

							std::istringstream ss( float_array.content );

							weights.reserve( count );

							for( size_t i = 0; i < count; ++i )
							{
								float weight = 0.0f;
								ss >> weight;
								weights.push_back( weight );
							}
						}
					}

					size_t vertex_weight_count = 0;
					{
						std::istringstream ss( std::string( vertex_weights.Attribute( "count" ) ) );
						ss >> vertex_weight_count;
					}

					std::vector< size_t > vcounts;
					vcounts.reserve( vertex_weight_count );
					{
						std::istringstream ss( vertex_weights[ "vcount" ].content );

						for( size_t i = 0; i < vertex_weight_count; ++i )
						{
							size_t count = 0;
							ss >> count;
							vcounts.push_back( count );
						}
					}

					std::istringstream ss( vertex_weights[ "v" ].content );

					for( size_t i = 0; i < vcounts.size(); ++i )
					{
						using WeightPair = std::pair< int, float >;

						size_t vcount = vcounts[ i ];

						std::vector< WeightPair > weight_pairs;
						for( size_t v = 0; v < vcount; ++v )
						{
							int    joint_index  = 0;
							size_t weight_index = 0;

							ss >> joint_index;
							ss >> weight_index;

							weight_pairs.push_back( { joint_index, weights[ weight_index ] } );
						}

						if( vcount > 4 )
						{
							std::sort( weight_pairs.begin(), weight_pairs.end(), []( const WeightPair& a, const WeightPair& b ) { return ( a.second > b.second ); } );

							float weight_to_redistribute = 0.0f;
							for( size_t v = 4; v < vcount; ++v )
								weight_to_redistribute += weight_pairs[ v ].second;

							for( size_t v = 0; v < 4; ++v )
								weight_pairs[ v ].second += ( weight_to_redistribute / 4 );

							weight_pairs.resize( 4 );
							vcount = 4;
						}

						int*   joint_ids_write = reinterpret_cast< int*   >( &vertex_data[ i * vertex_stride + joint_ids_offset ] );
						float* weights_write   = reinterpret_cast< float* >( &vertex_data[ i * vertex_stride + weights_offset ] );

						for( size_t v = 0; v < vcount; ++v )
						{
							joint_ids_write[ v ] = weight_pairs[ v ].first;
							weights_write[ v ]   = weight_pairs[ v ].second;
						}
					}

					break;
				}
			}

			IndexFormat index_format;
			switch( index_size )
			{
				case 1: { index_format = IndexFormat::Byte;       } break;
				case 2: { index_format = IndexFormat::Word;       } break;
				case 4: { index_format = IndexFormat::DoubleWord; } break;
			}

			mesh.vertex_buffer = std::make_unique< VertexBuffer >( vertex_data.get(), vertex_count, vertex_stride );
			mesh.index_buffer  = std::make_unique< IndexBuffer >( index_format, index_data.get(), face_count * 3 );

			m_meshes.emplace_back( std::move( mesh ) );
		}
	}

	const XMLElement& node = collada[ "library_visual_scenes" ][ "visual_scene" ][ "node" ];

	if( !node.children.empty() )
	{
		const XMLElement& root_node = node[ "node" ];
		size_t            next_id   = 0;

		m_root_joint       = std::make_unique< Joint >();
		m_root_joint->id   = ( next_id++ );
		m_root_joint->name = root_node.Attribute( "name" );

		ColladaParseNodeRecursive( root_node, Matrix4(), *m_root_joint, next_id );
	}

	return true;
}

bool Model::ParseOBJ( ByteSpan data, const VertexLayout& layout )
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

	/* Opt out if not OBJ */
	if( vertex_count == 0 && face_count == 0 )
		return false;

	uint8_t index_size = 4;
	/**/ if( vertex_count < std::numeric_limits< uint8_t  >::max() ) { index_size = 1; }
	else if( vertex_count < std::numeric_limits< uint16_t >::max() ) { index_size = 2; }

	size_t stride          = layout.GetStride();
	size_t pos_offset      = layout.OffsetOf( VertexComponent::Position );
	size_t normal_offset   = layout.OffsetOf( VertexComponent::Normal );
	size_t color_offset    = layout.OffsetOf( VertexComponent::Color );
	size_t texcoord_offset = layout.OffsetOf( VertexComponent::TexCoord );
	auto   vertex_data     = std::unique_ptr< uint8_t[] >( new uint8_t[ stride * vertex_count ] );
	auto   index_data      = std::unique_ptr< uint8_t[] >( new uint8_t[ index_size * face_count * 3 ] );

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
			uint32_t indices[ 3 ];

			if( std::sscanf( it, "f %u %u %u\n%n", &indices[ 0 ], &indices[ 1 ], &indices[ 2 ], &bytes_read ) == 3 )
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
			uint32_t indices[ 3 ];

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
			}

			const Orbit::Vector3* positions[ 3 ]
			{
				reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * indices[ 0 ] + pos_offset ] ),
				reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * indices[ 1 ] + pos_offset ] ),
				reinterpret_cast< const Orbit::Vector3* >( &vertex_data[ stride * indices[ 2 ] + pos_offset ] ),
			};

			const Orbit::Vector3 pos0_to_pos1 = ( *positions[ 1 ] - *positions[ 0 ] );
			const Orbit::Vector3 pos0_to_pos2 = ( *positions[ 2 ] - *positions[ 0 ] );

			Orbit::Vector3 normal = pos0_to_pos1.CrossProduct( pos0_to_pos2 );
			normal.Normalize();

			Orbit::Vector3* normal0_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * indices[ 0 ] + normal_offset ] );
			Orbit::Vector3* normal1_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * indices[ 1 ] + normal_offset ] );
			Orbit::Vector3* normal2_write = reinterpret_cast< Orbit::Vector3* >( &vertex_data[ stride * indices[ 2 ] + normal_offset ] );

			*normal0_write = normal;
			*normal1_write = normal;
			*normal2_write = normal;
		}
	}

	IndexFormat index_format;
	switch( index_size )
	{
		case 1: { index_format = IndexFormat::Byte;       } break;
		case 2: { index_format = IndexFormat::Word;       } break;
		case 4: { index_format = IndexFormat::DoubleWord; } break;
	}

	/* Create mesh */
	Mesh mesh;
	mesh.vertex_buffer = std::make_unique< VertexBuffer >( vertex_data.get(), vertex_count, stride );
	mesh.index_buffer  = std::make_unique< IndexBuffer >( index_format, index_data.get(), face_count * 3 );

	m_meshes.emplace_back( std::move( mesh ) );

	return true;
}

ORB_NAMESPACE_END
