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
	LibraryEffects();
	LibraryImages();
	LibraryGeometries( vertex_layout );
	LibraryAnimations();
	LibraryVisualScenes();
}

std::string_view COLLADAFile::SourceID( const XMLElement& element )
{
	std::string_view source_id = element.FindAttribute( "source" );

	// Remove leading '#'
	source_id.remove_prefix( 1 );

	return source_id;
}

void COLLADAFile::LibraryEffects( void )
{
	for( const XMLElement& elm_geometry : root_element_[ "COLLADA" ][ "library_effects" ] )
	{
	}
}

void COLLADAFile::LibraryImages( void )
{
	for( const XMLElement& elm_geometry : root_element_[ "COLLADA" ][ "library_images" ] )
	{
	}
}

void COLLADAFile::LibraryMaterials( void )
{
	for( const XMLElement& elm_geometry : root_element_[ "COLLADA" ][ "library_materials" ] )
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
		const XMLElement&      elm_input_vertex   = *elm_triangles.FindChildWithAttribute( "input", { "semantic", "VERTEX" } );
		const XMLElement&      elm_input_normal   = *elm_triangles.FindChildWithAttribute( "input", { "semantic", "NORMAL" } );
		const XMLElement&      elm_input_texcoord = *elm_triangles.FindChildWithAttribute( "input", { "semantic", "TEXCOORD" } );
		const std::string_view id_vertices        = SourceID( elm_input_vertex );
		const XMLElement&      elm_vertices       = *elm_mesh.FindChildWithAttribute( "vertices", { "id", id_vertices } );
		const XMLElement&      elm_input_position = *elm_vertices.FindChildWithAttribute( "input", { "semantic", "POSITION" } );
		const std::string_view id_positions       = SourceID( elm_input_position );
		const std::string_view id_normals         = SourceID( elm_input_normal );
		const std::string_view id_texcoords_0     = SourceID( elm_input_texcoord );

//////////////////////////////////////////////////////////////////////////

		// Indices
		std::vector< uint32_t > indices;
		{
			const std::string_view material_name       = elm_triangles.FindAttribute( "material" );
			const uint32_t         num_indices         = num_triangles * 3 * num_inputs;
			std::string_view       content_index_array = elm_triangles[ "p" ].content;

			for( uint32_t triangle_index = 0; triangle_index < num_indices; ++triangle_index )
			{
				const size_t           space        = content_index_array.find_first_of( ' ' );
				const std::string_view value_string = content_index_array.substr( 0, space );
				const uint32_t         value        = FromString< uint32_t >( value_string );

				// Store value in indices
				indices.push_back( value );

				// Chip away from the content string
				content_index_array.remove_prefix( space + 1 );
			}
		}

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

		if( vertex_layout.Contains( VertexComponent::Position ) )
		{
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

				// Add position to vector
				positions.push_back( position );
			}
		}

//////////////////////////////////////////////////////////////////////////

		std::vector< Vector3 > normals;

		if( vertex_layout.Contains( VertexComponent::Normal ) )
		{
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

				// Add normal to vector
				normals.emplace_back( std::move( normal ) );
			}
		}

//////////////////////////////////////////////////////////////////////////

		std::vector< Vector2 > texcoords;

		if( vertex_layout.Contains( VertexComponent::TexCoord ) )
		{
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

		const bool need_index_buffer = ( minimum_vertices_required < ( num_triangles * 3 ) );

		// Create indices, unless the amount of vertices match with amount of indices
		if( need_index_buffer ) geometry.Reserve( minimum_vertices_required, num_triangles );
		else                    geometry.Reserve( minimum_vertices_required, 0 );

//////////////////////////////////////////////////////////////////////////

		const size_t index_stride = num_inputs * 3;
		uint32_t     index_offset = 0;

		if( chosen_id == id_positions )
		{
			index_offset = FromString< uint32_t >( elm_input_vertex.FindAttribute( "offset" ) );

			for( const Vector4& position : positions )
			{
				Vertex vertex;
				vertex.position = position;
				geometry.AddVertex( vertex );
			}
		}
		else if( chosen_id == id_normals )
		{
			index_offset = FromString< uint32_t >( elm_input_normal.FindAttribute( "offset" ) );

			for( const Vector3& normal : normals )
			{
				Vertex vertex;
				vertex.normal = normal;
				geometry.AddVertex( vertex );
			}
		}
		else if( chosen_id == id_texcoords_0 )
		{
			index_offset = FromString< uint32_t >( elm_input_texcoord.FindAttribute( "offset" ) );

			for( const Vector2& texcoord : texcoords )
			{
				Vertex vertex;
				vertex.tex_coord = texcoord;
				geometry.AddVertex( vertex );
			}
		}

		if( need_index_buffer )
		{
			for( size_t i = 0; i < num_triangles; ++i )
			{
				Face face;
				face.indices[ 0 ] = indices[ index_offset + num_inputs * ( i * 3 + 0 ) ];
				face.indices[ 1 ] = indices[ index_offset + num_inputs * ( i * 3 + 1 ) ];
				face.indices[ 2 ] = indices[ index_offset + num_inputs * ( i * 3 + 2 ) ];
				geometry.AddFace( face );
			}
		}

		if( chosen_id != id_positions )
		{
			const uint32_t offset       = FromString< uint32_t >( elm_input_vertex.FindAttribute( "offset" ) );
			const size_t   num_vertices = geometry.GetVertexCount();

			for( size_t face_index = 0; face_index < num_triangles; ++face_index )
			{
				for( size_t i = 0; i < 3; ++i )
				{
					const uint32_t vertex_index = indices[ index_offset + num_inputs * ( face_index * 3 + i ) ];
					Vertex         vertex       = geometry.GetVertex( vertex_index );

					vertex.position             = positions[ indices[ offset + num_inputs * ( face_index * 3 + i ) ] ];

					geometry.SetVertex( vertex_index, vertex );
				}
			}
		}

		if( chosen_id != id_normals )
		{
			const uint32_t offset       = FromString< uint32_t >( elm_input_normal.FindAttribute( "offset" ) );
			const size_t   num_vertices = geometry.GetVertexCount();

			for( size_t face_index = 0; face_index < num_triangles; ++face_index )
			{
				for( size_t i = 0; i < 3; ++i )
				{
					const uint32_t vertex_index = indices[ index_offset + num_inputs * ( face_index * 3 + i ) ];
					Vertex         vertex       = geometry.GetVertex( vertex_index );

					vertex.normal               = normals[ indices[ offset + num_inputs * ( face_index * 3 + i ) ] ];

					geometry.SetVertex( vertex_index, vertex );
				}
			}
		}

		if( chosen_id != id_texcoords_0 )
		{
			const uint32_t offset       = FromString< uint32_t >( elm_input_texcoord.FindAttribute( "offset" ) );
			const size_t   num_vertices = geometry.GetVertexCount();

			for( size_t face_index = 0; face_index < num_triangles; ++face_index )
			{
				for( size_t i = 0; i < 3; ++i )
				{
					const uint32_t vertex_index = indices[ index_offset + num_inputs * ( face_index * 3 + i ) ];
					Vertex         vertex       = geometry.GetVertex( vertex_index );

					vertex.tex_coord            = texcoords[ indices[ offset + num_inputs * ( face_index * 3 + i ) ] ];

					geometry.SetVertex( vertex_index, vertex );
				}
			}
		}

//////////////////////////////////////////////////////////////////////////

		const std::string_view geometry_name = elm_geometry.FindAttribute( "name" );

		// Create mesh
		model_data_.meshes.emplace_back( geometry.ToMesh( geometry_name ) );
	}
}

void COLLADAFile::LibraryAnimations( void )
{
	for( const XMLElement& animation : root_element_[ "COLLADA" ][ "library_animations" ] )
	{
	}
}

void COLLADAFile::LibraryVisualScenes( void )
{
	for( const XMLElement& visual_scene : root_element_[ "COLLADA" ][ "library_visual_scenes" ] )
	{
	}
}

ORB_NAMESPACE_END
