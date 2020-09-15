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

#include "WavefrontOBJFile.h"

#include "Orbit/Graphics/Geometry/Geometry.h"
#include "Orbit/Graphics/ModelFormats/WavefrontMTLFile.h"

#include <cassert>
#include <cstdio>
#include <optional>

ORB_NAMESPACE_BEGIN

WavefrontOBJFile::WavefrontOBJFile( ByteSpan data, const VertexLayout& vertex_layout )
{
	Init( data.Size() );

	const char*                               src = reinterpret_cast< const char* >( data.Ptr() );
	std::optional< Geometry >                 current_geometry;
	std::string                               current_mesh_name = "Unnamed Wavefront Model";
	std::vector< Vector2 >                    tex_coords;
	std::vector< Vector3 >                    normals;
	std::vector< WavefrontMTLFile::Material > materials;

	while( !IsEOF() )
	{
		const std::string_view line = ReadLine( src );

		// Skip empty lines
		if( line.empty() )
			continue;

		// Skip comments
		if( line[ 0 ] == '#' )
			continue;

//////////////////////////////////////////////////////////////////////////

		char  string_buf[ 64 ];
		float x, y, z, w;
		float u, v;
		int   v1, v2, v3;
		int   vt1, vt2, vt3;
		int   vn1, vn2, vn3;

		if( std::sscanf( line.data(), "mtllib %s", string_buf ) > 0 )
		{
			const Asset            mtl_asset     = Asset( string_buf );
			const WavefrontMTLFile mtl_file      = WavefrontMTLFile( { mtl_asset.GetData(), mtl_asset.GetSize() } );
			auto                   new_materials = mtl_file.GetMaterials();

			// Store materials
			materials.insert( materials.end(), new_materials.begin(), new_materials.end() );
		}
		else if( std::sscanf( line.data(), "o %s", string_buf ) > 0 )
		{
			if( current_geometry )
				ProduceMesh( *current_geometry, current_mesh_name, tex_coords.empty(), normals.empty() );

			current_geometry  = std::make_optional< Geometry >( vertex_layout );
			current_mesh_name = string_buf;

			tex_coords.clear();
			normals.clear();
		}
		else if( w = 1.0f; std::sscanf( line.data(), "v %f %f %f %f", &x, &y, &z, &w ) >= 3 )
		{
			// Backup geometry if 'o' tag was missing
			if( !current_geometry )
				current_geometry = std::make_optional< Geometry >( vertex_layout );

			// Create vertex
			Vertex vertex;
			vertex.position.x = x;
			vertex.position.y = y;
			vertex.position.z = z;
			vertex.position.w = w;

			current_geometry->AddVertex( vertex );
		}
		else if( v = 0.0f, w = 0.0f; std::sscanf( line.data(), "vt %f %f %f", &u, &v, &w ) >= 1 )
		{
			tex_coords.emplace_back( u, v );
		}
		else if( std::sscanf( line.data(), "vn %f %f %f", &x, &y, &z ) == 3 )
		{
			normals.emplace_back( x, y, z );
		}
		else if( std::sscanf( line.data(), "f %d %d %d", &v1, &v2, &v3 ) == 3 )
		{
			assert( tex_coords.empty() );
			assert( normals.empty() );

			// Create face
			Face face;
			face.indices[ 0 ] = v1 - 1;
			face.indices[ 1 ] = v2 - 1;
			face.indices[ 2 ] = v3 - 1;

			current_geometry->AddFace( face );
		}
		else if( std::sscanf( line.data(), "f %d/%d %d/%d %d/%d", &v1, &vt1, &v2, &vt2, &v3, &vt3 ) == 6 )
		{
			assert( normals.empty() );

			// Fetch vertices
			Vertex vertex1 = current_geometry->GetVertex( v1 - 1 );
			Vertex vertex2 = current_geometry->GetVertex( v2 - 1 );
			Vertex vertex3 = current_geometry->GetVertex( v3 - 1 );

			// Update texture coordinates
			vertex1.tex_coord = tex_coords.at( vt1 - 1 );
			vertex2.tex_coord = tex_coords.at( vt2 - 1 );
			vertex3.tex_coord = tex_coords.at( vt3 - 1 );

			// Update geometry
			current_geometry->SetVertex( v1 - 1, vertex1 );
			current_geometry->SetVertex( v2 - 1, vertex2 );
			current_geometry->SetVertex( v3 - 1, vertex3 );

			// Create face
			Face face;
			face.indices[ 0 ] = v1 - 1;
			face.indices[ 1 ] = v2 - 1;
			face.indices[ 2 ] = v3 - 1;

			current_geometry->AddFace( face );
		}
		else if( std::sscanf( line.data(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3 ) == 9 )
		{
			// Fetch vertices
			Vertex vertex1 = current_geometry->GetVertex( v1 - 1 );
			Vertex vertex2 = current_geometry->GetVertex( v2 - 1 );
			Vertex vertex3 = current_geometry->GetVertex( v3 - 1 );

			// Update texture coordinates
			vertex1.tex_coord = tex_coords.at( vt1 - 1 );
			vertex2.tex_coord = tex_coords.at( vt2 - 1 );
			vertex3.tex_coord = tex_coords.at( vt3 - 1 );

			// Update normals
			vertex1.normal = normals.at( vn1 - 1 );
			vertex2.normal = normals.at( vn2 - 1 );
			vertex3.normal = normals.at( vn3 - 1 );

			// Update geometry
			current_geometry->SetVertex( v1 - 1, vertex1 );
			current_geometry->SetVertex( v2 - 1, vertex2 );
			current_geometry->SetVertex( v3 - 1, vertex3 );

			// Create face
			Face face;
			face.indices[ 0 ] = v1 - 1;
			face.indices[ 1 ] = v2 - 1;
			face.indices[ 2 ] = v3 - 1;

			current_geometry->AddFace( face );
		}
		else if( std::sscanf( line.data(), "f %d//%d %d//%d %d//%d", &v1, &vn1, &v2, &vn2, &v3, &vn3 ) == 6 )
		{
			assert( tex_coords.empty() );

			// Fetch vertices
			Vertex vertex1 = current_geometry->GetVertex( v1 - 1 );
			Vertex vertex2 = current_geometry->GetVertex( v2 - 1 );
			Vertex vertex3 = current_geometry->GetVertex( v3 - 1 );

			// Update normals
			vertex1.normal = normals.at( vn1 - 1 );
			vertex2.normal = normals.at( vn2 - 1 );
			vertex3.normal = normals.at( vn3 - 1 );

			// Update geometry
			current_geometry->SetVertex( v1 - 1, vertex1 );
			current_geometry->SetVertex( v2 - 1, vertex2 );
			current_geometry->SetVertex( v3 - 1, vertex3 );

			// Create face
			Face face;
			face.indices[ 0 ] = v1 - 1;
			face.indices[ 1 ] = v2 - 1;
			face.indices[ 2 ] = v3 - 1;

			current_geometry->AddFace( face );
		}
		else if( std::sscanf( line.data(), "usemtl %s", string_buf ) > 0 )
		{
			// #TODO: Use material in @materials
		}
		else if( line == "s 1" )
		{
			// #TODO: Turn on smooth shading
		}
		else if( line == "s off" )
		{
			// #TODO: Turn off smooth shading
		}
		else if( line.substr( 0, 2 ) == "g " )
		{
			assert( false && "Groups are not yet supported" );
		}
	}

	if( current_geometry )
		ProduceMesh( *current_geometry, current_mesh_name, tex_coords.empty(), normals.empty() );
}

void WavefrontOBJFile::ProduceMesh( Geometry& geometry, std::string_view mesh_name, bool generate_tex_coords, bool generate_normals )
{
	if( generate_tex_coords )
		geometry.GenerateTexCoords();

	if( generate_normals )
		geometry.GenerateNormals();

	meshes_.push_back( geometry.ToMesh( mesh_name ) );
}

ORB_NAMESPACE_END
