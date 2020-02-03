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

#include "OBJLoader.h"

#include "Orbit/Math/Vector3.h"
#include "Orbit/Math/Vector4.h"

ORB_NAMESPACE_BEGIN

bool OBJLoader::Init( ByteSpan data )
{
	m_data            = data;
	m_vertex_location = nullptr;
	m_face_location   = nullptr;

	for( const uint8_t* it = m_data.begin(); it < m_data.end(); )
	{
		if( it[ 0 ] == 'v' )
		{
			if( m_vertex_location == nullptr )
				m_vertex_location = it;
		}
		else if( it[ 0 ] == 'f' )
		{
			if( m_face_location == nullptr )
				m_face_location = it;
		}

		while( it < m_data.end() && *( it++ ) != '\n' );
	}

	return true;
}

size_t OBJLoader::PeekVertexCount( void )
{
	size_t count = 0;

	for( const uint8_t* it = m_data.begin(); it < m_data.end(); )
	{
		if( it[ 0 ] == 'v' )
			++count;

		while( it < m_data.end() && *( it++ ) != '\n' );
	}

	return count;
}

size_t OBJLoader::PeekFaceCount( void )
{
	size_t count = 0;

	for( const uint8_t* it = m_data.begin(); it < m_data.end(); )
	{
		if( it[ 0 ] == 'f' )
			++count;

		while( it < m_data.end() && *( it++ ) != '\n' );
	}

	return count;
}

size_t OBJLoader::PeekMeshCount( void )
{
	return 1;
}

void OBJLoader::ReadNextVertexPosition( Vector4* out )
{
	int bytes_read = 0;

	if( std::sscanf( reinterpret_cast< const char* >( m_vertex_location ), "v %f %f %f\n%n", &out->x, &out->y, &out->z, &bytes_read ) == 3 )
		m_vertex_location += bytes_read;
}

void OBJLoader::ReadNextVertexNormal( Vector3* out )
{
	/* TODO: Implement this */
	( void )out;
}

void OBJLoader::ReadNextFace( size_t* out )
{
	int bytes_read = 0;

	if( std::sscanf( reinterpret_cast< const char* >( m_face_location ), "f %zu %zu %zu\n%n", &out[ 0 ], &out[ 1 ], &out[ 2 ], &bytes_read ) == 3 )
	{
		out[ 0 ] -= 1;
		out[ 1 ] -= 1;
		out[ 2 ] -= 1;

		m_face_location += bytes_read;
	}
}

bool OBJLoader::ShouldGenerateNormals( void )
{
	return true;
}

ORB_NAMESPACE_END
