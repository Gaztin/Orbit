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

#include "COLLADAFile.h"

#include "Orbit/Core/Utility/StringConverting.h"
#include "Orbit/Graphics/Animation/KeyFrame.h"
#include "Orbit/Graphics/Geometry/Geometry.h"
#include "Orbit/Math/Matrix/Matrix4.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <sstream>

ORB_NAMESPACE_BEGIN

COLLADAFile::COLLADAFile( ByteSpan data, const VertexLayout& vertex_layout )
	: XMLFile( data )
{
	std::vector< std::string >      all_joint_names;
	std::vector< Matrix4 >          all_joint_transforms;
	std::map< std::string, size_t > mesh_id_table;

	for( const XMLElement& geometry : root_element_[ "COLLADA" ][ "library_geometries" ] )
	{
		if( geometry.name != "geometry" )
			continue;

		const std::string geometry_id = std::string( geometry.FindAttribute( "id" ) );
		const XMLElement& polylist    = geometry[ "mesh" ][ "polylist" ];

		size_t vertex_count = 0;
		/* Peek the number of vertices */
		if( const XMLElement* input = geometry[ "mesh" ][ "vertices" ].FindChildWithAttribute( "input", { "semantic", "POSITION" } ) )
		{
			const std::string positions_source( input->FindAttribute( "source" ).substr( 1 ) );

			if( const XMLElement* source = geometry[ "mesh" ].FindChildWithAttribute( "source", { "id", positions_source } ) )
			{
				std::istringstream ss( std::string( ( *source )[ "float_array" ].FindAttribute( "count" ) ) );
				ss >> vertex_count;
				assert( vertex_count % 3 == 0 );
				vertex_count /= 3;
			}
		}

		const size_t face_count = FromString< size_t >( polylist.FindAttribute( "count" ) );

		// Abort if not COLLADA
		if( vertex_count == 0 || face_count == 0 )
			return;

		std::vector< Vector4 > positions;
		std::vector< Vector3 > normals;
		std::vector< Vector2 > tex_coords;
		Geometry               mesh_geometry( vertex_layout );

		mesh_geometry.Reserve( vertex_count, face_count );

		if( vertex_layout.Contains( VertexComponent::Position ) )
		{
			if( const XMLElement* input = geometry[ "mesh" ][ "vertices" ].FindChildWithAttribute( "input", { "semantic", "POSITION" } ) )
			{
				const std::string source_id( input->FindAttribute( "source" ).substr( 1 ) );

				if( const XMLElement* source = geometry[ "mesh" ].FindChildWithAttribute( "source", { "id", source_id } ) )
				{
					const size_t       stride = FromString< size_t >( ( *source )[ "technique_common" ][ "accessor" ].FindAttribute( "stride" ) );
					std::istringstream ss     = std::istringstream( ( *source )[ "float_array" ].content );
					Vector4            vec    = Vector4( 0.0f, 0.0f, 0.0f, 1.0f );
					size_t             i      = 0;

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
		}

		if( vertex_layout.Contains( VertexComponent::Normal ) )
		{
			const size_t offset = vertex_layout.OffsetOf( VertexComponent::Normal );

			if( const XMLElement* input = polylist.FindChildWithAttribute( "input", { "semantic", "NORMAL" } ) )
			{
				const std::string source_id = std::string( input->FindAttribute( "source" ).substr( 1 ) );

				if( const XMLElement*  source = geometry[ "mesh" ].FindChildWithAttribute( "source", { "id", source_id } ) )
				{
					const size_t       stride = FromString< size_t >( ( *source )[ "technique_common" ][ "accessor" ].FindAttribute( "stride" ) );
					std::istringstream ss     = std::istringstream( ( *source )[ "float_array" ].content );
					Vector3            vec    = Vector3( 0.0f, 0.0f, 0.0f );
					size_t             i      = 0;

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
		}

		if( vertex_layout.Contains( VertexComponent::TexCoord ) )
		{
			const size_t offset = vertex_layout.OffsetOf( VertexComponent::TexCoord );

			if( const XMLElement* input = polylist.FindChildWithAttribute( "input", { "semantic", "TEXCOORD" } ) )
			{
				const std::string source_id = std::string( input->FindAttribute( "source" ).substr( 1 ) );

				if( const XMLElement* source = geometry[ "mesh" ].FindChildWithAttribute( "source", { "id", source_id } ) )
				{
					const size_t       stride = FromString< size_t >( ( *source )[ "technique_common" ][ "accessor" ].FindAttribute( "stride" ) );
					std::istringstream ss     = std::istringstream( ( *source )[ "float_array" ].content );
					Vector2            vec    = Vector2( 0.0f, 0.0f );
					size_t             i      = 0;

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
		}

		// Write to index data
		{
			const size_t          input_count = polylist.CountChildrenWithName( "input" );
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
				if( const XMLElement* input = polylist.FindChildWithAttribute( "input", { "semantic", "VERTEX" } ) )
				{
					size_t index = FromString< size_t >( input->FindAttribute( "offset" ) );

					for( size_t f = 0; f < face_count; ++f )
					{
						Face face{ };

						assert( vcounts[ f ] == 3 );

						for( size_t v = 0; v < vcounts[ f ]; ++v )
						{
							face.indices[ v ] = all_indices[ index ];
							index            += input_count;
						}

						mesh_geometry.AddFace( face );
					}
				}
			}

			for( size_t i = 0; i < vertex_count; ++i )
			{
				Vertex vertex   = { };
				vertex.position = positions[ i ];

				mesh_geometry.AddVertex( vertex );
			}

			if( const XMLElement* input = polylist.FindChildWithAttribute( "input", { "semantic", "NORMAL" } ) )
			{
				size_t normal_index = FromString< size_t >( input->FindAttribute( "offset" ) );

				for( Face face : mesh_geometry.GetFaces() )
				{
					for( size_t index : face.indices )
					{
						Vertex vertex = mesh_geometry.GetVertex( index );
						vertex.normal = normals[ all_indices[ normal_index ] ];
						normal_index += input_count;
					}
				}
			}

			if( const XMLElement* input = polylist.FindChildWithAttribute( "input", { "semantic", "TEXCOORD" } ) )
			{
				size_t tex_coord_index = FromString< size_t >( input->FindAttribute( "offset" ) );

				for( Face face : mesh_geometry.GetFaces() )
				{
					for( size_t index : face.indices )
					{
						Vertex vertex    = mesh_geometry.GetVertex( index );
						vertex.tex_coord = tex_coords[ all_indices[ index ] ];
						tex_coord_index += input_count;
					}
				}
			}
		}

		if( vertex_layout.Contains( VertexComponent::JointIDs ) || vertex_layout.Contains( VertexComponent::Weights ) )
		{
			for( const XMLElement& controller : root_element_[ "COLLADA" ][ "library_controllers" ] )
			{
				const std::string controller_id  = std::string( controller.FindAttribute( "id" ) );
				const XMLElement& skin           = controller[ "skin" ];
				std::string       skin_source_id = std::string( skin.FindAttribute( "source" ).substr( 1 ) );

				if( skin_source_id != geometry_id )
					continue;

				const XMLElement& vertex_weights = skin[ "vertex_weights" ];

				std::string weight_source_id;
				if( const XMLElement* input = vertex_weights.FindChildWithAttribute( "input", { "semantic", "WEIGHT" } ) )
					weight_source_id = input->FindAttribute( "source" ).substr( 1 );

				std::string joints_source_id;
				if( const XMLElement* input = skin[ "joints" ].FindChildWithAttribute( "input", { "semantic", "JOINT" } ) )
					joints_source_id = input->FindAttribute( "source" ).substr( 1 );

				std::string matrices_source_id;
				if( const XMLElement* input = skin[ "joints" ].FindChildWithAttribute( "input", { "semantic", "INV_BIND_MATRIX" } ) )
					matrices_source_id = input->FindAttribute( "source" ).substr( 1 );

				Matrix4 bind_shape_matrix;
				{
					std::istringstream ss( skin[ "bind_shape_matrix" ].content );

					for( size_t i = 0; i < 16; ++i )
						ss >> bind_shape_matrix[ i ];
				}

				std::vector< float > weights;
				for( const XMLElement& source : skin )
				{
					const std::string source_id( source.FindAttribute( "id" ) );

					if( source_id == weight_source_id )
					{
						const XMLElement& float_array = source[ "float_array" ];

						size_t count = 0;
						{
							std::istringstream ss( std::string( float_array.FindAttribute( "count" ) ) );
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
							std::istringstream ss( std::string( name_array.FindAttribute( "count" ) ) );
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
							std::istringstream ss( std::string( float_array.FindAttribute( "count" ) ) );
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
					std::istringstream ss( std::string( vertex_weights.FindAttribute( "count" ) ) );
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

						weight_pairs.emplace_back( joint_index, weights[ weight_index ] );
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

					Vertex vertex = mesh_geometry.GetVertex( i );

					for( size_t v = 0; v < vcount; ++v )
					{
						vertex.joint_ids[ v ] = weight_pairs[ v ].first;
						vertex.weights  [ v ] = weight_pairs[ v ].second;
					}

					mesh_geometry.SetVertex( i, vertex );
				}

				break;
			}
		}

//////////////////////////////////////////////////////////////////////////

		// Generate normals
		mesh_geometry.GenerateNormals();

//////////////////////////////////////////////////////////////////////////

		auto mesh = mesh_geometry.ToMesh( geometry.FindAttribute( "name" ) );

		// Add mesh to list
		model_data_.meshes.emplace_back( std::move( mesh ) );

		mesh_id_table[ "#" + geometry_id ] = model_data_.meshes.size();
	}

//////////////////////////////////////////////////////////////////////////

	for( const XMLElement& animation : root_element_[ "COLLADA" ][ "library_animations" ] )
	{
		if( animation.name != "animation" )
			continue;

		const XMLElement& sampler = animation[ "sampler" ];

		std::string input_source_id;
		if( const XMLElement* input = sampler.FindChildWithAttribute( "input", { "semantic", "INPUT" } ) )
		{
			std::istringstream ss( std::string( input->FindAttribute( "source" ) ) );
			ss.ignore( 1 );
			ss >> input_source_id;
		}

		std::string output_source_id;
		if( const XMLElement* input = sampler.FindChildWithAttribute( "input", { "semantic", "OUTPUT" } ) )
		{
			std::istringstream ss( std::string( input->FindAttribute( "source" ) ) );
			ss.ignore( 1 );
			ss >> output_source_id;
		}

		std::string interpolation_source_id;
		if( const XMLElement* input = sampler.FindChildWithAttribute( "input", { "semantic", "INTERPOLATION" } ) )
		{
			std::istringstream ss( std::string( input->FindAttribute( "source" ) ) );
			ss.ignore( 1 );
			ss >> interpolation_source_id;
		}

		std::string target_joint;
		{
			std::istringstream ss( std::string( animation[ "channel" ].FindAttribute( "target" ) ) );
			ss >> target_joint;
			target_joint.erase( target_joint.rfind( "/" ) );
		}

		std::vector< KeyFrame > key_frames;
		for( const XMLElement& source : animation )
		{
			if( source.name != "source" )
				continue;

			if( key_frames.empty() )
			{
				size_t count = 0;
				{
					std::istringstream ss( std::string( source[ "technique_common" ][ "accessor" ].FindAttribute( "count" ) ) );
					ss >> count;
				}

				key_frames.resize( count );
			}

			const std::string source_id( source.FindAttribute( "id" ) );

			if( source_id == input_source_id )
			{
				std::istringstream ss( source[ "float_array" ].content );

				for( KeyFrame& kf : key_frames )
					ss >> kf.time;
			}
			else if( source_id == output_source_id )
			{
				std::istringstream ss( source[ "float_array" ].content );

				for( KeyFrame& kf : key_frames )
				{
					for( size_t e = 0; e < 16; ++e )
						ss >> kf.transform[ e ];
				}
			}
			else if( source_id == interpolation_source_id )
			{
				std::istringstream ss( source[ "Name_array" ].content );

				/* One of: LINEAR, BEZIER, CARDINAL, HERMITE, BSPLINE and STEP */
				for( KeyFrame& kf : key_frames )
					ss >> kf.interpolation_type;
			}
		}

		// Sort keyframes
		std::sort( key_frames.begin(), key_frames.end(), []( const KeyFrame& a, const KeyFrame& b ) { return a.time < b.time; } );

		// Store keyframe
		animation_data_.keyframes.try_emplace( std::move( target_joint ), std::move( key_frames ) );
	}

//////////////////////////////////////////////////////////////////////////

	const XMLElement& visual_scene  = root_element_[ "COLLADA" ][ "library_visual_scenes" ][ "visual_scene" ];
	bool              has_animation = false;

	for( const XMLElement& node : visual_scene )
	{
		if( const XMLElement* instance_geometry = node.FindChild( "instance_geometry" ) )
		{
			if( auto it = mesh_id_table.find( std::string( instance_geometry->FindAttribute( "url" ) ) ); it != mesh_id_table.end() )
			{
				if( it->second >= model_data_.meshes.size() )
					continue;

				std::istringstream ss( node[ "matrix" ].content );

				for( size_t e = 0; e < 16; ++e )
					ss >> model_data_.meshes[ it->second ]->transform_[ e ];
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
			has_animation = true;
		}
	}

	// Parse joint hierarchy
	if( has_animation )
		model_data_.root_joint = ColladaParseNodeRecursive( visual_scene, Matrix4(), all_joint_names, all_joint_transforms );
}

Joint COLLADAFile::ColladaParseNodeRecursive( const XMLElement& node, const Matrix4& parent_inverse_bind_transform, const std::vector< std::string >& all_joint_names, const std::vector< Matrix4 >& all_joint_transforms )
{
	Joint joint;
	joint.name = node.FindAttribute( "id" );
	
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

ORB_NAMESPACE_END
