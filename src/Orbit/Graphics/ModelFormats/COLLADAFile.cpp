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
	Asset();
	LibraryEffects();
	LibraryImages();
	LibraryGeometries( vertex_layout );
	LibraryAnimations();
	LibraryVisualScenes();
}

void COLLADAFile::Asset( void )
{
	const XMLElement&      elm_asset = root_element_[ "COLLADA" ][ "asset" ];
	const std::string_view up_axis   = elm_asset[ "up_axis" ].content;

	// Set up correction matrix based on which axis is upwards
	/**/ if( up_axis == "X_UP" ) correction_matrix_.RotateZ( -Pi / 2 );
	else if( up_axis == "Y_UP" ) correction_matrix_.SetIdentity();
	else if( up_axis == "Z_UP" ) correction_matrix_.RotateX( -Pi / 2 );
}

void COLLADAFile::LibraryEffects( void )
{
	for( const XMLElement& elm_effect : root_element_[ "COLLADA" ][ "library_effects" ] )
	{
	}
}

void COLLADAFile::LibraryImages( void )
{
	for( const XMLElement& elm_image : root_element_[ "COLLADA" ][ "library_images" ] )
	{
	}
}

void COLLADAFile::LibraryMaterials( void )
{
	for( const XMLElement& elm_material : root_element_[ "COLLADA" ][ "library_materials" ] )
	{
	}
}

void COLLADAFile::LibraryGeometries( const VertexLayout& vertex_layout )
{
	for( const XMLElement& elm_geometry : root_element_[ "COLLADA" ][ "library_geometries" ] )
	{
		Geometry               geometry           = Geometry( vertex_layout );
		const XMLElement&      elm_mesh           = elm_geometry[ "mesh" ];
		const XMLElement&      elm_triangles      = *Or( elm_mesh.FindChild( "triangles" ), elm_mesh.FindChild( "polylist" ) );
		const uint32_t         num_triangles      = FromString< uint32_t >( elm_triangles.FindAttribute( "count" ) );
		const uint32_t         num_inputs         = elm_triangles.CountChildrenWithName( "input" );
		const XMLElement*      elm_input_vertex   = elm_triangles.FindChildWithAttribute( "input", { "semantic", "VERTEX" } );
		const XMLElement*      elm_input_normal   = elm_triangles.FindChildWithAttribute( "input", { "semantic", "NORMAL" } );
		const XMLElement*      elm_input_texcoord = elm_triangles.FindChildWithAttribute( "input", { "semantic", "TEXCOORD" } );

//////////////////////////////////////////////////////////////////////////

		// Find the largest amount of vertices required
		uint32_t         minimum_vertices_required = 0;
		std::string_view chosen_id;

		for( const XMLElement& source : elm_mesh )
		{
			if( source.name == "source" )
			{
				const XMLElement& accessor = source[ "technique_common" ][ "accessor" ];
				const uint32_t    count    = FromString< uint32_t >( accessor.FindAttribute( "count" ) );

				if( count > minimum_vertices_required )
				{
					minimum_vertices_required = count;
					chosen_id                 = source.FindAttribute( "id" );
				}
			}
		}

//////////////////////////////////////////////////////////////////////////

		std::vector< Vector4 > positions;

		if( elm_input_vertex && vertex_layout.Contains( VertexComponent::Position ) )
		{
			const std::string_view id_vertices  = SourceID( *elm_input_vertex );
			const XMLElement&      elm_vertices = *elm_mesh.FindChildWithAttribute( "vertices", { "id", id_vertices } );

			if( const XMLElement* elm_input_position = elm_vertices.FindChildWithAttribute( "input", { "semantic", "POSITION" } ) )
			{
				const std::string_view id_positions         = SourceID( *elm_input_position );
				const XMLElement&      elm_source_positions = *elm_mesh.FindChildWithAttribute( "source", { "id", id_positions } );
				const XMLElement&      elm_accessor         = elm_source_positions[ "technique_common" ][ "accessor" ];
				const uint32_t         count                = FromString< uint32_t >( elm_accessor.FindAttribute( "count" ) );
				const uint32_t         stride               = FromString< uint32_t >( elm_accessor.FindAttribute( "stride" ) );
				const std::string_view id_positions_array   = SourceID( elm_accessor );
				const XMLElement&      elm_float_array      = *elm_source_positions.FindChildWithAttribute( "float_array", { "id", id_positions_array } );
				std::string_view       stream               = elm_float_array.content;

				assert( stride <= 4 ); // Maximum 4 components

				for( uint32_t position_index = 0; position_index < count; ++position_index )
				{
					Vector4 position( 0, 0, 0, 1 );

					for( uint32_t component_index = 0; component_index < stride; ++component_index )
					{
						const size_t           space        = stream.find_first_of( ' ' );
						const std::string_view value_string = stream.substr( 0, space );
						const float            value        = FromString< float >( value_string );

						// Store values in position
						position[ component_index ] = value;

						// Chip away from the content string
						stream.remove_prefix( space + 1 );
					}

					// Correct point vector
					position = correction_matrix_ * position;

					// Add position to vector
					positions.push_back( position );
				}
			}
		}

//////////////////////////////////////////////////////////////////////////

		std::vector< Vector3 > normals;

		if( elm_input_normal && vertex_layout.Contains( VertexComponent::Normal ) )
		{
			const std::string_view id_normals       = SourceID( *elm_input_normal );
			const XMLElement&      elm_source       = *elm_mesh.FindChildWithAttribute( "source", { "id", id_normals } );
			const XMLElement&      elm_accessor     = elm_source[ "technique_common" ][ "accessor" ];
			const std::string_view id_normals_array = SourceID( elm_accessor );
			const uint32_t         count            = FromString< uint32_t >( elm_accessor.FindAttribute( "count" ) );
			const uint32_t         stride           = FromString< uint32_t >( elm_accessor.FindAttribute( "stride" ) );
			const XMLElement&      elm_float_array  = *elm_source.FindChildWithAttribute( "float_array", { "id", id_normals_array } );
			std::string_view       stream           = elm_float_array.content;

			assert( stride <= 3 ); // Maximum 3 components

			for( uint32_t normal_index = 0; normal_index < count; ++normal_index )
			{
				Vector3 normal;

				for( uint32_t component_index = 0; component_index < stride; ++component_index )
				{
					const size_t           space        = stream.find_first_of( ' ' );
					const std::string_view value_string = stream.substr( 0, space );
					const float            value        = FromString< float >( value_string );

					// Store values in normal
					normal[ component_index ] = value;

					// Chip away from the content string
					stream.remove_prefix( space + 1 );
				}

				// Correct unit vector
				normal = Vector3( correction_matrix_ * Vector4( normal, 0 ) );

				// Add normal to vector
				normals.emplace_back( std::move( normal ) );
			}
		}

//////////////////////////////////////////////////////////////////////////

		std::vector< Vector2 > texcoords;

		if( elm_input_texcoord && vertex_layout.Contains( VertexComponent::TexCoord ) )
		{
			const std::string_view id_texcoords_0  = SourceID( *elm_input_texcoord );
			const XMLElement&      elm_source      = *elm_mesh.FindChildWithAttribute( "source", { "id", id_texcoords_0 } );
			const XMLElement&      elm_accessor    = elm_source[ "technique_common" ][ "accessor" ];
			const std::string_view id_map_0_array  = SourceID( elm_accessor );
			const uint32_t         count           = FromString< uint32_t >( elm_accessor.FindAttribute( "count" ) );
			const uint32_t         stride          = FromString< uint32_t >( elm_accessor.FindAttribute( "stride" ) );
			const XMLElement&      elm_float_array = *elm_source.FindChildWithAttribute( "float_array", { "id", id_map_0_array } );
			std::string_view       stream          = elm_float_array.content;

			assert( stride <= 2 ); // Maximum 2 components

			for( uint32_t texcoord_index = 0; texcoord_index < count; ++texcoord_index )
			{
				Vector2 texcoord;

				for( uint32_t component_index = 0; component_index < stride; ++component_index )
				{
					const size_t           space        = stream.find_first_of( ' ' );
					const std::string_view value_string = stream.substr( 0, space );
					const float            value        = FromString< float >( value_string );

					// Store values in normal
					texcoord[ component_index ] = value;

					// Chip away from the content string
					stream.remove_prefix( space + 1 );
				}

				// Add texture coordinate to vector
				texcoords.emplace_back( std::move( texcoord ) );
			}
		}

//////////////////////////////////////////////////////////////////////////

		std::string_view content_p = elm_triangles[ "p" ].content;

		while( !content_p.empty() )
		{
			Vertex vertex;

			for( uint32_t input_offset = 0; input_offset < num_inputs; ++input_offset )
			{
				const size_t           space                        = content_p.find_first_of( ' ' );
				const uint32_t         index                        = FromString< uint32_t >( content_p.substr( 0, space ) );
				const std::string      current_input_offset_string  = std::to_string( input_offset );
				const XMLElement&      elm_input_for_current_offset = *elm_triangles.FindChildWithAttribute( "input", { "offset", current_input_offset_string } );
				const std::string_view semantic                     = elm_input_for_current_offset.FindAttribute( "semantic" );

				/**/ if( semantic == "VERTEX" )   { vertex.position  = positions.at( index ); }
				else if( semantic == "NORMAL" )   { vertex.normal    = normals  .at( index ); }
				else if( semantic == "TEXCOORD" ) { vertex.tex_coord = texcoords.at( index ); }

				// Chip away on the content string until it reaches the end
				if( space == std::string_view::npos ) content_p.remove_prefix( content_p.length() );
				else                                  content_p.remove_prefix( space + 1 );
			}

			// Create vertex
			geometry.AddVertex( vertex );
		}

//////////////////////////////////////////////////////////////////////////

		const std::string_view geometry_name = elm_geometry.FindAttribute( "name" );

		// Create mesh
		model_data_.meshes.emplace_back( geometry.ToMesh( geometry_name ) );
	}
}

void COLLADAFile::LibraryControllers( void )
{
	for( const XMLElement& elm_controller : root_element_[ "COLLADA" ][ "library_controllers" ] )
	{
	}
}

void COLLADAFile::LibraryAnimations( void )
{
	for( const XMLElement& elm_animation : root_element_[ "COLLADA" ][ "library_animations" ] )
	{
		const std::string_view         id     = elm_animation.FindAttribute( "id" );
		const std::string_view         name   = elm_animation.FindAttribute( "name" );
		const std::string              target = std::string( name ) + "/matrix";
		std::map< uint32_t, KeyFrame > key_frames;

		if( const XMLElement* elm_channel = elm_animation.FindChildWithAttribute( "channel", { "target", target } ) )
		{
			const std::string_view matrix_id   = SourceID( *elm_channel );
			const XMLElement&      elm_sampler = *elm_animation.FindChildWithAttribute( "sampler", { "id", matrix_id } );

			// INPUT
			if( const XMLElement* elm_input = elm_sampler.FindChildWithAttribute( "input", { "semantic", "INPUT" } ) )
			{
				const std::string_view input_id        = SourceID( *elm_input );
				const XMLElement&      elm_source      = *elm_animation.FindChildWithAttribute( "source", { "id", input_id } );
				const XMLElement&      elm_accessor    = elm_source[ "technique_common" ][ "accessor" ];
				const std::string_view input_array_id  = SourceID( elm_accessor );
				const XMLElement&      elm_float_array = *elm_source.FindChildWithAttribute( "float_array", { "id", input_array_id } );
				const uint32_t         count           = FromString< uint32_t >( elm_accessor.FindAttribute( "count" ) );
				std::string_view       content         = elm_float_array.content;

				for( uint32_t i = 0; i < count; ++i )
				{
					const size_t space = content.find_first_of( ' ' );
					const double time  = FromString< double >( content.substr( 0, space ) );

					// Give time to key frame
					key_frames[ i ].time = time;

					// Chip away on content string until it is empty
					content.remove_prefix( space + 1 );
				}
			}

			// OUTPUT
			if( const XMLElement* elm_input = elm_sampler.FindChildWithAttribute( "input", { "semantic", "OUTPUT" } ) )
			{
				const std::string_view input_id        = SourceID( *elm_input );
				const XMLElement&      elm_source      = *elm_animation.FindChildWithAttribute( "source", { "id", input_id } );
				const XMLElement&      elm_accessor    = elm_source[ "technique_common" ][ "accessor" ];
				const std::string_view input_array_id  = SourceID( elm_accessor );
				const XMLElement&      elm_float_array = *elm_source.FindChildWithAttribute( "float_array", { "id", input_array_id } );
				const uint32_t         count           = FromString< uint32_t >( elm_accessor.FindAttribute( "count" ) );
				const uint32_t         stride          = FromString< uint32_t >( elm_accessor.FindAttribute( "stride" ) );
				std::string_view       content         = elm_float_array.content;

				for( uint32_t i = 0; i < count; ++i )
				{
					for( uint32_t j = 0; j < stride; ++j )
					{
						const size_t space   = content.find_first_of( ' ' );
						const float  element = FromString< float >( content.substr( 0, space ) );

						// Apply element to transform
						key_frames[ i ].transform[ j ] = element;

						// Chip away on content string until it is empty
						content.remove_prefix( space + 1 );
					}

					// Correct transform
					key_frames[ i ].transform *= correction_matrix_;
				}
			}

			// INTERPOLATION
			if( const XMLElement* elm_input = elm_sampler.FindChildWithAttribute( "input", { "semantic", "INTERPOLATION" } ) )
			{
				const std::string_view input_id        = SourceID( *elm_input );
				const XMLElement&      elm_source      = *elm_animation.FindChildWithAttribute( "source", { "id", input_id } );
				const XMLElement&      elm_accessor    = elm_source[ "technique_common" ][ "accessor" ];
				const std::string_view input_array_id  = SourceID( elm_accessor );
				const XMLElement&      elm_name_array  = *elm_source.FindChildWithAttribute( "Name_array", { "id", input_array_id } );
				const uint32_t         count           = FromString< uint32_t >( elm_accessor.FindAttribute( "count" ) );
				std::string_view       content         = elm_name_array.content;

				for( uint32_t i = 0; i < count; ++i )
				{
					const size_t           space              = content.find_first_of( ' ' );
					const std::string_view interpolation_type = content.substr( 0, space );

					// Set interpolation type of key frame
					key_frames[ i ].interpolation_type = interpolation_type;

					// Chip away on content string until it is empty
					content.remove_prefix( space + 1 );
				}
			}
		}
	}
}

void COLLADAFile::LibraryVisualScenes( void )
{
	for( const XMLElement& elm_visual_scene : root_element_[ "COLLADA" ][ "library_visual_scenes" ] )
	{
	}
}

std::string_view COLLADAFile::SourceID( const XMLElement& element )
{
	std::string_view source_id = element.FindAttribute( "source" );

	// Remove leading '#'
	source_id.remove_prefix( 1 );

	return source_id;
}

ORB_NAMESPACE_END
