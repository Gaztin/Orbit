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

#include "Model.h"

#include "Orbit/Core/IO/Parser/XML/XMLParser.h"
#include "Orbit/Core/IO/Log.h"
#include "Orbit/Core/Utility/Color.h"
#include "Orbit/Core/Utility/StringConverting.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Geometry/GeometryData.h"
#include "Orbit/Math/Vector/Vector2.h"
#include "Orbit/Math/Vector/Vector3.h"
#include "Orbit/Math/Vector/Vector4.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <vector>

ORB_NAMESPACE_BEGIN

Model::Model( ByteSpan data, const VertexLayout& layout )
{
	if( !( ParseCollada( data, layout ) ||
	       ParseOBJ( data, layout ) ) )
	{
		LogError( "Failed to load model. Unsupported format." );
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

//////////////////////////////////////////////////////////////////////////

	const XMLElement&               collada = xml_parser.GetRootElement()[ "COLLADA" ];
	std::vector< std::string >      all_joint_names;
	std::vector< Matrix4 >          all_joint_transforms;
	std::map< std::string, size_t > mesh_id_table;

	for( const XMLElement& geometry : collada[ "library_geometries" ] )
	{
		if( geometry.name != "geometry" )
			continue;

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

		const size_t face_count = FromString< size_t >( polylist.Attribute( "count" ) );

		/* Opt out if not COLLADA */
		if( vertex_count == 0 || face_count == 0 )
			return false;

		GeometryData           geometry_data( layout );
		std::vector< Vector4 > positions;
		std::vector< Vector3 > normals;
		std::vector< Vector2 > tex_coords;

		geometry_data.Reserve( vertex_count, face_count );

		if( layout.Contains( VertexComponent::Position ) )
		{
			std::string source_id( geometry[ "mesh" ][ "vertices" ].ChildWithAttribute( "input", "semantic", "POSITION" ).Attribute( "source" ) );

			if( !source_id.empty() )
			{
				source_id.erase( source_id.begin() );

				const XMLElement&  source = geometry[ "mesh" ].ChildWithAttribute( "source", "id", source_id );
				const size_t       stride = FromString< size_t >( source[ "technique_common" ][ "accessor" ].Attribute( "stride" ) );
				std::istringstream ss( source[ "float_array" ].content );
				Vector4            vec( 0.0f, 0.0f, 0.0f, 1.0f );
				size_t             i = 0;

				while( ss >> vec[ i++ ] )
				{
					if( i == stride )
					{
						positions.push_back( vec );
						i = 0;
					}
				}
			}
		}

		if( layout.Contains( VertexComponent::Normal ) )
		{
			const size_t offset = layout.OffsetOf( VertexComponent::Normal );
			std::string  source_id( polylist.ChildWithAttribute( "input", "semantic", "NORMAL" ).Attribute( "source" ) );

			if( !source_id.empty() )
			{
				source_id.erase( source_id.begin() );

				const XMLElement&  source = geometry[ "mesh" ].ChildWithAttribute( "source", "id", source_id );
				const size_t       stride = FromString< size_t >( source[ "technique_common" ][ "accessor" ].Attribute( "stride" ) );
				std::istringstream ss( source[ "float_array" ].content );
				Vector3            vec( 0.0f, 0.0f, 0.0f );
				size_t             i = 0;

				while( ss >> vec[ i++ ] )
				{
					if( i == stride )
					{
						normals.push_back( vec );
						i = 0;
					}
				}
			}
		}

		if( layout.Contains( VertexComponent::TexCoord ) )
		{
			const size_t offset = layout.OffsetOf( VertexComponent::TexCoord );
			std::string  source_id( polylist.ChildWithAttribute( "input", "semantic", "TEXCOORD" ).Attribute( "source" ) );

			if( !source_id.empty() )
			{
				source_id.erase( source_id.begin() );

				const XMLElement&  source = geometry[ "mesh" ].ChildWithAttribute( "source", "id", source_id );
				const size_t       stride = FromString< size_t >( source[ "technique_common" ][ "accessor" ].Attribute( "stride" ) );
				std::istringstream ss( source[ "float_array" ].content );
				Vector2            vec( 0.0f, 0.0f );
				size_t             i = 0;

				while( ss >> vec[ i++ ] )
				{
					if( i == stride )
					{
						tex_coords.push_back( vec );
						i = 0;
					}
				}
			}
		}

		/* Write to index data */
		{
			const size_t input_count = polylist.CountChildren( "input" );

			std::vector< size_t > all_indices;
			{
				std::istringstream ss( polylist[ "p" ].content );
				size_t             index;

				while( ss >> index )
					all_indices.push_back( index );
			}

			std::vector< size_t > vcounts;
			{
				std::istringstream ss( polylist[ "vcount" ].content );
				size_t             index;

				while( ss >> index )
					vcounts.push_back( index );
			}

			{
				const XMLElement& input = polylist.ChildWithAttribute( "input", "semantic", "VERTEX" );
				size_t            index = FromString< size_t >( input.Attribute( "offset" ) );

				for( size_t f = 0; f < face_count; ++f )
				{
					Face face{ };

					assert( vcounts[ f ] == 3 );

					for( size_t v = 0; v < vcounts[ f ]; ++v )
					{
						face.indices[ v ] = all_indices[ index ];
						index            += input_count;
					}

					geometry_data.AddFace( face );
				}
			}

			for( size_t i = 0; i < vertex_count; ++i )
			{
				Vertex vertex{ };
				vertex.position = positions[ i ];

				geometry_data.AddVertex( vertex );
			}

			if( auto& input = polylist.ChildWithAttribute( "input", "semantic", "NORMAL" ); input.IsValid() )
			{
				size_t normal_index = FromString< size_t >( input.Attribute( "offset" ) );

				for( Face face : geometry_data.GetFaces() )
				{
					for( size_t index : face.indices )
					{
						Vertex vertex = geometry_data.GetVertex( index );
						vertex.normal = normals[ all_indices[ normal_index ] ];
						normal_index += input_count;
					}
				}
			}

			if( auto& input = polylist.ChildWithAttribute( "input", "semantic", "TEXCOORD" ); input.IsValid() )
			{
				size_t tex_coord_index = FromString< size_t >( input.Attribute( "offset" ) );

				for( Face face : geometry_data.GetFaces() )
				{
					for( size_t index : face.indices )
					{
						Vertex vertex    = geometry_data.GetVertex( index );
						vertex.tex_coord = tex_coords[ all_indices[ index ] ];
						tex_coord_index += input_count;
					}
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

					Vertex vertex = geometry_data.GetVertex( i );

					for( size_t v = 0; v < vcount; ++v )
					{
						vertex.joint_ids[ v ] = weight_pairs[ v ].first;
						vertex.weights  [ v ] = weight_pairs[ v ].second;
					}

					geometry_data.SetVertex( i, vertex );
				}

				break;
			}
		}

		geometry_data.GenerateNormals();

//////////////////////////////////////////////////////////////////////////

		Mesh mesh = geometry_data.ToMesh( geometry.Attribute( "name" ) );
		meshes_.emplace_back( std::move( mesh ) );

		mesh_id_table[ "#" + geometry_id ] = meshes_.size();
	}

//////////////////////////////////////////////////////////////////////////

	const XMLElement& visual_scene   = collada[ "library_visual_scenes" ][ "visual_scene" ];
	bool              has_joint_data = false;

	for( const XMLElement& node : visual_scene )
	{
		if( const XMLElement& instance_geometry = node[ "instance_geometry" ]; instance_geometry.IsValid() )
		{
			if( auto it = mesh_id_table.find( std::string( instance_geometry.Attribute( "url" ) ) ); it != mesh_id_table.end() )
			{
				std::istringstream ss( node[ "matrix" ].content );

				for( size_t e = 0; e < 16; ++e )
					ss >> meshes_[ it->second ].transform[ e ];
			}
			else
			{
				// Failed to find mesh in ID table
				assert( false );
			}
		}
		else if( node.children.size() == 1 )
		{
			// Treat the lack of an "instance_xxx" element as being a joint node
			has_joint_data = true;
		}
	}

	if( has_joint_data )
		root_joint_ = std::make_unique< Joint >( ColladaParseNodeRecursive( visual_scene, Matrix4(), all_joint_names, all_joint_transforms ) );

	return true;
}

bool Model::ParseOBJ( ByteSpan data, const VertexLayout& layout )
{
	const char* begin        = reinterpret_cast< const char* >( data.begin() );
	const char* end          = reinterpret_cast< const char* >( data.end() );
	const char* it           = begin;
	size_t      vertex_count = 0;
	size_t      face_count   = 0;

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

//////////////////////////////////////////////////////////////////////////

	GeometryData geometry_data( layout );

	geometry_data.Reserve( vertex_count, face_count );

	/* Parse the vertex and index data */
	while( it < end )
	{
		Vertex vertex;
		Face   face;

		if( std::sscanf( it, "v %f %f %f\n", &vertex.position[ 0 ], &vertex.position[ 1 ], &vertex.position[ 2 ] ) == 3 )
			geometry_data.AddVertex( vertex );

		else if( std::sscanf( it, "f %zd %zd %zd\n", &face.indices[ 0 ], &face.indices[ 1 ], &face.indices[ 2 ] ) == 3 )
			geometry_data.AddFace( Face{ ( face.indices[ 0 ] - 1 ), ( face.indices[ 1 ] - 1 ), ( face.indices[ 2 ] - 1 ) } );

		/* Seek to next line */
		while( it < end && *( it++ ) != '\n' );
	}

	geometry_data.GenerateNormals();

//////////////////////////////////////////////////////////////////////////

	Mesh mesh = geometry_data.ToMesh( "OBJRoot" );
	meshes_.emplace_back( std::move( mesh ) );

	return true;
}

ORB_NAMESPACE_END
