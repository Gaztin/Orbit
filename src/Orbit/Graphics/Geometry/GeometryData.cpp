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

#include "GeometryData.h"

#include "Orbit/Core/Debug/Trace.h"
#include "Orbit/Graphics/Geometry/Face.h"
#include "Orbit/Graphics/Geometry/Vertex.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

GeometryData::GeometryData( const VertexLayout& vertex_layout )
	: vertex_layout_( vertex_layout )
	, index_size_   ( 0 )
{
}

GeometryData::GeometryData( GeometryData&& other )
	: vertex_layout_( std::move( other.vertex_layout_ ) )
	, vertex_data_  ( std::move( other.vertex_data_ ) )
	, face_data_    ( std::move( other.face_data_ ) )
	, index_size_   ( other.index_size_ )
{
	other.index_size_ = 0;
}

void GeometryData::Reserve( size_t vertex_count, size_t face_count )
{
	index_size_ = EvalIndexSize( vertex_count );
	vertex_data_.reserve( vertex_layout_.GetStride() * vertex_count );
	face_data_.reserve( index_size_ * face_count );
}

size_t GeometryData::AddFace( const Face& face )
{
	const size_t highest_face_index = *std::max_element( face.indices.begin(), face.indices.end() );

	if( const uint8_t new_index_size = EvalIndexSize( highest_face_index ); new_index_size > index_size_ )
		UpgradeFaceData( new_index_size );

//////////////////////////////////////////////////////////////////////////

	const size_t face_size = ( index_size_ * 3 );
	const size_t index     = GetFaceCount();

	face_data_.resize( face_data_.size() + face_size );

//////////////////////////////////////////////////////////////////////////

	void* dst = &face_data_[ index * face_size ];

	switch( index_size_ )
	{
		default:
		case 1:
		{
			static_cast< uint8_t* >( dst )[ 0 ] = static_cast< uint8_t >( face.indices[ 0 ] );
			static_cast< uint8_t* >( dst )[ 1 ] = static_cast< uint8_t >( face.indices[ 1 ] );
			static_cast< uint8_t* >( dst )[ 2 ] = static_cast< uint8_t >( face.indices[ 2 ] );

		} break;

		case 2:
		{
			static_cast< uint16_t* >( dst )[ 0 ] = static_cast< uint16_t >( face.indices[ 0 ] );
			static_cast< uint16_t* >( dst )[ 1 ] = static_cast< uint16_t >( face.indices[ 1 ] );
			static_cast< uint16_t* >( dst )[ 2 ] = static_cast< uint16_t >( face.indices[ 2 ] );

		} break;

		case 4:
		{
			static_cast< uint32_t* >( dst )[ 0 ] = static_cast< uint32_t >( face.indices[ 0 ] );
			static_cast< uint32_t* >( dst )[ 1 ] = static_cast< uint32_t >( face.indices[ 1 ] );
			static_cast< uint32_t* >( dst )[ 2 ] = static_cast< uint32_t >( face.indices[ 2 ] );

		} break;
	}

	return index;
}

size_t GeometryData::AddVertex( const Vertex& vertex )
{
	const size_t stride           = vertex_layout_.GetStride();
	const size_t old_vertex_count = GetVertexCount();

	// Do we need to upgrade our index buffer?
	if( const uint8_t new_index_size = EvalIndexSize( old_vertex_count + 1 ); new_index_size > index_size_ )
		UpgradeFaceData( new_index_size );

	vertex_data_.resize( vertex_data_.size() + stride );

	SetVertex( old_vertex_count, vertex );

	return old_vertex_count;
}

void GeometryData::SetVertex( size_t index, const Vertex& vertex )
{
	uint8_t* dst = &vertex_data_[ index * vertex_layout_.GetStride() ];

	if( vertex_layout_.Contains( VertexComponent::Position ) ) memcpy( dst + vertex_layout_.OffsetOf( VertexComponent::Position ), &vertex.position,  sizeof( Vector4 )     );
	if( vertex_layout_.Contains( VertexComponent::Normal ) )   memcpy( dst + vertex_layout_.OffsetOf( VertexComponent::Normal ),   &vertex.normal,    sizeof( Vector3 )     );
	if( vertex_layout_.Contains( VertexComponent::Color ) )    memcpy( dst + vertex_layout_.OffsetOf( VertexComponent::Color ),    &vertex.color,     sizeof( Color   )     );
	if( vertex_layout_.Contains( VertexComponent::TexCoord ) ) memcpy( dst + vertex_layout_.OffsetOf( VertexComponent::TexCoord ), &vertex.tex_coord, sizeof( Vector2 )     );
	if( vertex_layout_.Contains( VertexComponent::JointIDs ) ) memcpy( dst + vertex_layout_.OffsetOf( VertexComponent::JointIDs ), &vertex.joint_ids, sizeof( int     ) * 4 );
	if( vertex_layout_.Contains( VertexComponent::Weights ) )  memcpy( dst + vertex_layout_.OffsetOf( VertexComponent::Weights ),  &vertex.weights,   sizeof( float   ) * 4 );
}

void GeometryData::GenerateNormals( void )
{
	for( Face face : GetFaces() )
	{
		const Vertex triangle_vertices[ 3 ]
		{
			GetVertex( face.indices[ 0 ] ),
			GetVertex( face.indices[ 1 ] ),
			GetVertex( face.indices[ 2 ] ),
		};

		const Orbit::Vector3 pos0_to_pos1 = Vector3( triangle_vertices[ 1 ].position - triangle_vertices[ 0 ].position );
		const Orbit::Vector3 pos0_to_pos2 = Vector3( triangle_vertices[ 2 ].position - triangle_vertices[ 0 ].position );
		const Orbit::Vector3 normal       = ( pos0_to_pos1.CrossProduct( pos0_to_pos2 ) ).Normalized();

		for( size_t i = 0; i < 3; ++i )
		{
			Vertex vertex = triangle_vertices[ i ];
			vertex.normal = normal;

			SetVertex( face.indices[ i ], vertex );
		}
	}
}

size_t GeometryData::GetVertexCount( void ) const
{
	return ( vertex_data_.size() / vertex_layout_.GetStride() );
}

size_t GeometryData::GetFaceCount( void ) const
{
	return ( ( face_data_.size() / 3 ) / index_size_ );
}

Vertex GeometryData::GetVertex( size_t index ) const
{
	const uint8_t* src = &vertex_data_[ index * vertex_layout_.GetStride() ];
	Vertex         vertex;

	if( vertex_layout_.Contains( VertexComponent::Position ) ) memcpy( &vertex.position,  src + vertex_layout_.OffsetOf( VertexComponent::Position ), sizeof( Vector4 )     );
	if( vertex_layout_.Contains( VertexComponent::Normal ) )   memcpy( &vertex.normal,    src + vertex_layout_.OffsetOf( VertexComponent::Normal ),   sizeof( Vector3 )     );
	if( vertex_layout_.Contains( VertexComponent::Color ) )    memcpy( &vertex.color,     src + vertex_layout_.OffsetOf( VertexComponent::Color ),    sizeof( Color   )     );
	if( vertex_layout_.Contains( VertexComponent::TexCoord ) ) memcpy( &vertex.tex_coord, src + vertex_layout_.OffsetOf( VertexComponent::TexCoord ), sizeof( Vector2 )     );
	if( vertex_layout_.Contains( VertexComponent::JointIDs ) ) memcpy( &vertex.joint_ids, src + vertex_layout_.OffsetOf( VertexComponent::JointIDs ), sizeof( int     ) * 4 );
	if( vertex_layout_.Contains( VertexComponent::Weights ) )  memcpy( &vertex.weights,   src + vertex_layout_.OffsetOf( VertexComponent::Weights ),  sizeof( float   ) * 4 );

	return vertex;
}

Face GeometryData::GetFace( size_t index ) const
{
	Face face;

	for( size_t i = 0; i < 3; ++i )
		memcpy( &face.indices[ i ], &face_data_[ ( index * 3 + i ) * index_size_ ], index_size_ );

	return face;
}

FaceRange GeometryData::GetFaces( void ) const
{
	return FaceRange( this );
}

VertexRange GeometryData::GetVertices( void ) const
{
	return VertexRange( this );
}

Mesh GeometryData::ToMesh( void ) const
{
	const size_t vertex_stride = vertex_layout_.GetStride();
	const size_t vertex_count  = GetVertexCount();
	const size_t index_size    = EvalIndexSize( vertex_count );
	const size_t index_count   = ( face_data_.size() / index_size );
	Mesh         mesh;

	if( !vertex_data_.empty() )
		mesh.vertex_buffer = std::make_unique< VertexBuffer >( vertex_data_.data(), vertex_count, vertex_stride );

	if( !face_data_.empty() )
		mesh.index_buffer = std::make_unique< IndexBuffer >( GetIndexFormat(), face_data_.data(), index_count );

	return mesh;
}

GeometryData& GeometryData::operator=( GeometryData&& other )
{
	vertex_layout_ = std::move( other.vertex_layout_ );
	vertex_data_   = std::move( other.vertex_data_ );
	face_data_     = std::move( other.face_data_ );

	return *this;
}

void GeometryData::UpgradeFaceData( uint8_t new_index_size )
{
	const size_t           old_index_size = index_size_;
	const size_t           index_count    = face_data_.size() / old_index_size;
	std::vector< uint8_t > new_face_data;

	ORB_TRACE( "Upgrading face data from index size %d to %d.", old_index_size, new_index_size );

	new_face_data.resize( index_count * new_index_size );

	for( size_t i = 0; i < index_count; ++i )
		std::memcpy( &new_face_data[ i * new_index_size ], &face_data_[ i * old_index_size ], old_index_size );

	face_data_  = std::move( new_face_data );
	index_size_ = new_index_size;
}

uint8_t GeometryData::EvalIndexSize( size_t index_or_vertex_count ) const
{
	if( index_or_vertex_count <= 1 )
		return 1;

#if( !ORB_HAS_D3D11 )

	if( index_or_vertex_count <= std::numeric_limits< uint8_t >::max() )
		return 1;

#endif // !ORB_HAS_D3D11

	if( index_or_vertex_count <= std::numeric_limits< uint16_t >::max() )
		return 2;

	if( index_or_vertex_count <= std::numeric_limits< uint32_t >::max() )
		return 4;

	return 0;
}

IndexFormat GeometryData::GetIndexFormat( void ) const
{
	switch( index_size_ )
	{
		default:
		case 1: { return IndexFormat::Byte; }
		case 2: { return IndexFormat::Word; }
		case 4: { return IndexFormat::DoubleWord; }
	}
}

ORB_NAMESPACE_END
