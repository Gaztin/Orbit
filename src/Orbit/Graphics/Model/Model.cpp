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

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "Orbit/Core/IO/Parser/XML/XMLParser.h"
#include "Orbit/Core/IO/Log.h"
#include "Orbit/Core/Utility/Color.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Math/Vector2.h"
#include "Orbit/Math/Vector3.h"
#include "Orbit/Math/Vector4.h"

ORB_NAMESPACE_BEGIN

Model::Model( ByteSpan data, const VertexLayout& layout )
{
	if( !( ParseCollada( data, layout ) ||
	       ParseOBJ( data, layout ) ) )
	{
		LogError( "Failed to load model. Unsupported format." );
	}
}

void Model::ClearVertexData( uint8_t* vertex_data, size_t vertex_count, const VertexLayout& vertex_layout )
{
	const size_t stride = vertex_layout.GetStride();

	if( vertex_layout.Contains( VertexComponent::Position ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::Position );

		for( size_t i = 0; i < vertex_count; ++i )
		{
			Vector4* w = reinterpret_cast< Vector4* >( &vertex_data[ stride * i + offset ] );
			*w = Vector4( 0.0f, 0.0f, 0.0f, 1.0f );
		}
	}

	if( vertex_layout.Contains( VertexComponent::Normal ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::Normal );

		for( size_t i = 0; i < vertex_count; ++i )
		{
			Vector3* w = reinterpret_cast< Vector3* >( &vertex_data[ stride * i + offset ] );
			*w = Vector3( 0.0f, 0.0f, 1.0f );
		}
	}

	if( vertex_layout.Contains( VertexComponent::Color ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::Color );

		for( size_t i = 0; i < vertex_count; ++i )
		{
			Color* w = reinterpret_cast< Color* >( &vertex_data[ stride * i + offset ] );
			*w = Color( 0.5f, 0.5f, 0.5f, 1.0f );
		}
	}

	if( vertex_layout.Contains( VertexComponent::TexCoord ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::TexCoord );

		for( size_t i = 0; i < vertex_count; ++i )
		{
			Vector2* w = reinterpret_cast< Vector2* >( &vertex_data[ stride * i + offset ] );
			*w = Vector2( 0.0f, 0.0f );
		}
	}

	if( vertex_layout.Contains( VertexComponent::JointIDs ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::JointIDs );

		for( size_t i = 0; i < vertex_count; ++i )
		{
			int* joint_ids_write = reinterpret_cast< int* >( &vertex_data[ stride * i + offset ] );
			joint_ids_write[ 0 ] = 0;
			joint_ids_write[ 1 ] = 0;
			joint_ids_write[ 2 ] = 0;
			joint_ids_write[ 3 ] = 0;
		}
	}

	if( vertex_layout.Contains( VertexComponent::Weights ) )
	{
		const size_t offset = vertex_layout.OffsetOf( VertexComponent::Weights );

		for( size_t i = 0; i < vertex_count; ++i )
		{
			float* weights_write = reinterpret_cast< float* >( &vertex_data[ stride * i + offset ] );
			weights_write[ 0 ] = 1.0f;
			weights_write[ 1 ] = 0.0f;
			weights_write[ 2 ] = 0.0f;
			weights_write[ 3 ] = 0.0f;
		}
	}
}

void Model::WriteIndexHelper( uint8_t* index_data, size_t index_size, size_t index, size_t value )
{
	switch( index_size )
	{
		case 1:
		{
			uint8_t* w = &index_data[ index ];
			*w = static_cast< uint8_t >( value );
		} break;

		case 2:
		{
			uint16_t* w = reinterpret_cast< uint16_t* >( &index_data[ index * 2 ] );
			*w = static_cast< uint16_t >( value );
		} break;

		case 4:
		{
			uint32_t* w = reinterpret_cast< uint32_t* >( &index_data[ index * 4 ] );
			*w = static_cast< uint32_t >( value );
		} break;
	}
}

size_t Model::ReadIndexHelper( const uint8_t* index_data, size_t index_size, size_t index )
{
	switch( index_size )
	{
		case 1:
		{
			const uint8_t* r = &index_data[ index ];
			return static_cast< size_t >( *r );
		}

		case 2:
		{
			const uint16_t* r = reinterpret_cast< const uint16_t* >( &index_data[ index * 2 ] );
			return static_cast< size_t >( *r );
		}

		case 4:
		{
			const uint32_t* r = reinterpret_cast< const uint32_t* >( &index_data[ index * 4 ] );
			return static_cast< size_t >( *r );
		}

		default:
		{
			return static_cast< size_t >( ~0 );
		}
	}
}

void Model::GenerateNormals( uint8_t* vertex_data, const uint8_t* index_data, size_t face_count, size_t index_size, const VertexLayout& vertex_layout )
{
	if( !vertex_layout.Contains( VertexComponent::Position ) || !vertex_layout.Contains( VertexComponent::Normal ) )
		return;

	const size_t stride        = vertex_layout.GetStride();
	const size_t pos_offset    = vertex_layout.OffsetOf( VertexComponent::Position );
	const size_t normal_offset = vertex_layout.OffsetOf( VertexComponent::Normal );

	for( uint32_t face = 0; face < face_count; ++face )
	{
		const size_t triangle_indices[ 3 ]
		{
			ReadIndexHelper( index_data, index_size, ( face * 3 + 0 ) ),
			ReadIndexHelper( index_data, index_size, ( face * 3 + 1 ) ),
			ReadIndexHelper( index_data, index_size, ( face * 3 + 2 ) ),
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

size_t Model::EvalIndexSize( size_t vertex_count )
{

#if( !ORB_HAS_D3D11 )

	if( vertex_count <= std::numeric_limits< uint8_t >::max() )
		return 1;

#endif // !ORB_HAS_D3D11

	if( vertex_count <= std::numeric_limits< uint16_t >::max() )
		return 2;

	if( vertex_count <= std::numeric_limits< uint32_t >::max() )
		return 4;

	return 0;
}

IndexFormat Model::EvalIndexFormat( size_t index_size )
{
	switch( index_size )
	{
		case 1:  return IndexFormat::Byte;
		case 2:  return IndexFormat::Word;
		case 4:  return IndexFormat::DoubleWord;
		default: return static_cast< IndexFormat >( ~0 );
	}
}

static Joint ColladaParseNodeRecursive( const XMLElement& node, const Matrix4& parent_inverse_bind_transform, const std::vector< std::string >& all_joint_names, const std::vector< Matrix4 >& all_joint_transforms )
{
	Joint joint;
	joint.name = node.Attribute( "id" );
	
	auto find_joint = [ &joint ]( const std::string& str ) { return str == joint.name; };

	if( auto it = std::find_if( all_joint_names.begin(), all_joint_names.end(), find_joint ); it != all_joint_names.end() )
	{
		joint.id                     = static_cast< int >( it - all_joint_names.begin() );
		joint.inverse_bind_transform = all_joint_transforms[ joint.id ];
	}
	else
	{
		std::istringstream ss( node[ "matrix" ].content );
		Matrix4            local_bind_transform;

		joint.id = -1;

		for( size_t i = 0; i < 16; ++i )
			ss >> local_bind_transform[ i ];

		const Matrix4 parent_bind_transform = parent_inverse_bind_transform.Inverted();
		const Matrix4 bind_transform        = ( parent_bind_transform * local_bind_transform );

		joint.inverse_bind_transform = bind_transform.Inverted();
	}

	for( const XMLElement& child : node )
	{
		if( child.name != "node" )
			continue;

		joint.children.push_back( ColladaParseNodeRecursive( child, joint.inverse_bind_transform, all_joint_names, all_joint_transforms ) );
	}

	return joint;
}

bool Model::ParseCollada( ByteSpan data, const VertexLayout& layout )
{
	const XMLParser xml_parser( data );

	if( !xml_parser.IsGood() )
		return false;

	const XMLElement&          collada = xml_parser.GetRootElement()[ "COLLADA" ];
	std::vector< std::string > all_joint_names;
	std::vector< Matrix4 >     all_joint_transforms;

	for( const XMLElement& geometry : collada[ "library_geometries" ] )
	{
		if( geometry.name != "geometry" )
			continue;

		Mesh mesh;
		mesh.name = geometry.Attribute( "name" );

		const std::string geometry_id( geometry.Attribute( "id" ) );
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

		const size_t index_size    = EvalIndexSize( vertex_count );
		const size_t vertex_stride = layout.GetStride();
		auto         vertex_data   = std::unique_ptr< uint8_t[] >( new uint8_t[ vertex_stride * vertex_count ] );
		auto         index_data    = std::unique_ptr< uint8_t[] >( new uint8_t[ index_size * face_count * 3 ] );

		ClearVertexData( vertex_data.get(), vertex_count, layout );

		if( layout.Contains( VertexComponent::Position ) )
		{
			const size_t offset = layout.OffsetOf( VertexComponent::Position );
			std::string  source_id( geometry[ "mesh" ][ "vertices" ].ChildWithAttribute( "input", "semantic", "POSITION" ).Attribute( "source" ) );
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
				Vector4* w = reinterpret_cast< Vector4* >( &vertex_data[ i * vertex_stride + offset ] );

				for( size_t c = 0; c < stride; ++c )
					ss >> ( *w )[ c ];
			}
		}

		if( layout.Contains( VertexComponent::Normal ) )
		{
			const size_t offset = layout.OffsetOf( VertexComponent::Normal );
			std::string  source_id( polylist.ChildWithAttribute( "input", "semantic", "NORMAL" ).Attribute( "source" ) );
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
				Vector3* w = reinterpret_cast< Vector3* >( &vertex_data[ i * vertex_stride + offset ] );

				for( size_t c = 0; c < stride; ++c )
					ss >> ( *w )[ c ];
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
				size_t index = ~0ul;
				ss >> index;

				WriteIndexHelper( index_data.get(), index_size, i, index );

				for( size_t input = 1; input < input_count; ++input )
				{
					size_t dummy;
					ss >> dummy;

					assert( dummy == index );
				}
			}
		}

		if( layout.Contains( VertexComponent::JointIDs ) || layout.Contains( VertexComponent::Weights ) )
		{
			for( const XMLElement& controller : collada[ "library_controllers" ] )
			{
				const std::string controller_id( controller.Attribute( "id" ) );
				const XMLElement& skin = controller[ "skin" ];
				std::string       skin_source_id( skin.Attribute( "source" ) );
				skin_source_id.erase( skin_source_id.begin() );

				if( skin_source_id != geometry_id )
					continue;

				const XMLElement& vertex_weights = skin[ "vertex_weights" ];
				std::string       weight_source_id( vertex_weights.ChildWithAttribute( "input", "semantic", "WEIGHT" ).Attribute( "source" ) );
				std::string       joints_source_id( skin[ "joints" ].ChildWithAttribute( "input", "semantic", "JOINT" ).Attribute( "source" ) );
				std::string       matrices_source_id( skin[ "joints" ].ChildWithAttribute( "input", "semantic", "INV_BIND_MATRIX" ).Attribute( "source" ) );
				weight_source_id.erase( weight_source_id.begin() );
				joints_source_id.erase( joints_source_id.begin() );
				matrices_source_id.erase( matrices_source_id.begin() );

				Matrix4 bind_shape_matrix;
				{
					std::istringstream ss( skin[ "bind_shape_matrix" ].content );

					for( size_t i = 0; i < 16; ++i )
						ss >> bind_shape_matrix[ i ];
				}

				std::vector< float > weights;
				for( const XMLElement& source : skin )
				{
					const std::string source_id( source.Attribute( "id" ) );

					if( source_id == weight_source_id )
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
					else if( source_id == joints_source_id )
					{
						const XMLElement& name_array = source[ "Name_array" ];

						size_t count = 0;
						{
							std::istringstream ss( std::string( name_array.Attribute( "count" ) ) );
							ss >> count;
						}

						if( all_joint_names.size() != count )
						{
							std::istringstream ss( name_array.content );

							all_joint_names.clear();
							all_joint_names.reserve( count );

							for( size_t i = 0; i < count; ++i )
							{
								std::string joint_name;
								ss >> joint_name;
								all_joint_names.emplace_back( std::move( joint_name ) );
							}
						}
					}
					else if( source_id == matrices_source_id )
					{
						const XMLElement& float_array = source[ "float_array" ];

						size_t count = 0;
						{
							std::istringstream ss( std::string( float_array.Attribute( "count" ) ) );
							ss >> count;
							assert( count % 16 == 0 );
							count /= 16;
						}

						if( all_joint_transforms.size() != count )
						{
							std::istringstream ss( float_array.content );

							all_joint_transforms.clear();
							all_joint_transforms.reserve( count );

							for( size_t i = 0; i < count; ++i )
							{
								Matrix4 joint_transform;
								for( size_t e = 0; e < 16; ++e )
									ss >> joint_transform[ e ];

								all_joint_transforms.push_back( joint_transform * bind_shape_matrix );
							}
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
						int    joint_index = 0;
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

					if( layout.Contains( VertexComponent::JointIDs ) )
					{
						const size_t offset = layout.OffsetOf( VertexComponent::JointIDs );
						int*         w      = reinterpret_cast< int* >( &vertex_data[ i * vertex_stride + offset ] );

						for( size_t v = 0; v < vcount; ++v )
							w[ v ] = weight_pairs[ v ].first;
					}

					if( layout.Contains( VertexComponent::Weights ) )
					{
						const size_t offset = layout.OffsetOf( VertexComponent::Weights );
						float*       w      = reinterpret_cast< float* >( &vertex_data[ i * vertex_stride + offset ] );

						for( size_t v = 0; v < vcount; ++v )
							w[ v ] = weight_pairs[ v ].second;
					}
				}

				break;
			}
		}

		const IndexFormat index_format = EvalIndexFormat( index_size );

		mesh.vertex_buffer = std::make_unique< VertexBuffer >( vertex_data.get(), vertex_count, vertex_stride );
		mesh.index_buffer  = std::make_unique< IndexBuffer >( index_format, index_data.get(), face_count * 3 );

		m_meshes.emplace_back( std::move( mesh ) );
	}

	const XMLElement& visual_scene = collada[ "library_visual_scenes" ][ "visual_scene" ];

	if( !visual_scene.children.empty() )
		m_root_joint = std::make_unique< Joint >( ColladaParseNodeRecursive( visual_scene, Matrix4(), all_joint_names, all_joint_transforms ) );

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

	const size_t index_size  = EvalIndexSize( vertex_count );
	const size_t stride      = layout.GetStride();
	auto         vertex_data = std::unique_ptr< uint8_t[] >( new uint8_t[ stride * vertex_count ] );
	auto         index_data  = std::unique_ptr< uint8_t[] >( new uint8_t[ index_size * face_count * 3 ] );

	ClearVertexData( vertex_data.get(), vertex_count, layout );

	uint32_t vertices_read  = 0;
	uint32_t faces_read     = 0;

	/* Parse the vertex and index data */
	while( it < end )
	{
		int bytes_read;

		/* Vertex position */
		if( layout.Contains( VertexComponent::Position ) )
		{
			const size_t offset = layout.OffsetOf( VertexComponent::Position );
			float        pos[ 3 ];

			if( std::sscanf( it, "v %f %f %f\n%n", &pos[ 0 ], &pos[ 1 ], &pos[ 2 ], &bytes_read ) == 3 )
			{
				Vector4* w = reinterpret_cast< Vector4* >( &vertex_data[ stride * vertices_read + offset ] );
				*w = Vector4( pos[ 0 ], pos[ 1 ], pos[ 2 ], 1.0f );

				++vertices_read;
				it += bytes_read;

				continue;
			}
		}

		/* Face indices */
		{
			size_t indices[ 3 ];

			if( std::sscanf( it, "f %zd %zd %zd\n%n", &indices[ 0 ], &indices[ 1 ], &indices[ 2 ], &bytes_read ) == 3 )
			{
				for( size_t i = 0; i < 3; ++i )
					WriteIndexHelper( index_data.get(), index_size, ( ( faces_read * 3 ) + i ), ( indices[ i ] - 1 ) );

				++faces_read;
				it += bytes_read;

				continue;
			}
		}

		/* Seek to next line */
		while( it < end && *( it++ ) != '\n' );
	}

	GenerateNormals( vertex_data.get(), index_data.get(), face_count, index_size, layout );

	const IndexFormat index_format = EvalIndexFormat( index_size );

	Mesh mesh;
	mesh.vertex_buffer = std::make_unique< VertexBuffer >( vertex_data.get(), vertex_count, stride );
	mesh.index_buffer  = std::make_unique< IndexBuffer >( index_format, index_data.get(), face_count * 3 );

	m_meshes.emplace_back( std::move( mesh ) );

	return true;
}

ORB_NAMESPACE_END
