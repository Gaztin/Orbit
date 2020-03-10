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

#include "Orbit/Graphics/Geometry/Face.h"
#include "Orbit/Graphics/Geometry/Vertex.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

GeometryData::GeometryData( size_t max_vertex_count, const VertexLayout& vertex_layout )
	: vertex_layout_( vertex_layout )
	, index_size_   ( EvalIndexSize( max_vertex_count ) )
{
}

void GeometryData::Reserve( size_t vertex_count, size_t face_count )
{
	vertex_data_.reserve( vertex_layout_.GetStride() * vertex_count );
	face_data_.reserve( index_size_ * face_count );
}

void GeometryData::AddFace( const Face& face )
{
	const size_t face_size = ( index_size_ * 3 );

	face_data_.resize( face_data_.size() + face_size );

//////////////////////////////////////////////////////////////////////////

	void* dst = &face_data_[ face_data_.size() - face_size ];

	switch( index_size_ )
	{
		default:
		case 1:
		{
			reinterpret_cast< uint8_t* >( dst )[ 0 ] = static_cast< uint8_t >( face.indices[ 0 ] );
			reinterpret_cast< uint8_t* >( dst )[ 1 ] = static_cast< uint8_t >( face.indices[ 1 ] );
			reinterpret_cast< uint8_t* >( dst )[ 2 ] = static_cast< uint8_t >( face.indices[ 2 ] );

		} break;

		case 2:
		{
			reinterpret_cast< uint16_t* >( dst )[ 0 ] = static_cast< uint16_t >( face.indices[ 0 ] );
			reinterpret_cast< uint16_t* >( dst )[ 1 ] = static_cast< uint16_t >( face.indices[ 1 ] );
			reinterpret_cast< uint16_t* >( dst )[ 2 ] = static_cast< uint16_t >( face.indices[ 2 ] );

		} break;

		case 4:
		{
			reinterpret_cast< uint32_t* >( dst )[ 0 ] = static_cast< uint32_t >( face.indices[ 0 ] );
			reinterpret_cast< uint32_t* >( dst )[ 1 ] = static_cast< uint32_t >( face.indices[ 1 ] );
			reinterpret_cast< uint32_t* >( dst )[ 2 ] = static_cast< uint32_t >( face.indices[ 2 ] );

		} break;
	}
}

void GeometryData::AddVertex( const Vertex& vertex )
{
	const size_t stride = vertex_layout_.GetStride();

	vertex_data_.resize( vertex_data_.size() + stride );

	SetVertex( ( ( vertex_data_.size() / stride ) - 1 ), vertex );
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

Mesh GeometryData::ToMesh( void )
{
	const size_t vertex_stride = vertex_layout_.GetStride();
	Mesh         mesh;

	if( !vertex_data_.empty() )
		mesh.vertex_buffer = std::make_unique< VertexBuffer >( vertex_data_.data(), ( vertex_data_.size() / vertex_stride ), vertex_stride );

	if( !face_data_.empty() )
		mesh.index_buffer = std::make_unique< IndexBuffer >( EvalIndexFormat(), face_data_.data(), ( face_data_.size() / index_size_ ) );

	return mesh;
}

Vertex GeometryData::GetVertex( size_t index ) const
{
	const uint8_t* src = &vertex_data_[ index * vertex_layout_.GetStride() ];
	Vertex         vertex{ };

	if( vertex_layout_.Contains( VertexComponent::Position ) ) memcpy( &vertex.position,  src + vertex_layout_.OffsetOf( VertexComponent::Position ), sizeof( Vector4 )     );
	if( vertex_layout_.Contains( VertexComponent::Normal ) )   memcpy( &vertex.normal,    src + vertex_layout_.OffsetOf( VertexComponent::Normal ),   sizeof( Vector3 )     );
	if( vertex_layout_.Contains( VertexComponent::Color ) )    memcpy( &vertex.color,     src + vertex_layout_.OffsetOf( VertexComponent::Color ),    sizeof( Color   )     );
	if( vertex_layout_.Contains( VertexComponent::TexCoord ) ) memcpy( &vertex.tex_coord, src + vertex_layout_.OffsetOf( VertexComponent::TexCoord ), sizeof( Vector2 )     );
	if( vertex_layout_.Contains( VertexComponent::JointIDs ) ) memcpy( &vertex.joint_ids, src + vertex_layout_.OffsetOf( VertexComponent::JointIDs ), sizeof( int     ) * 4 );
	if( vertex_layout_.Contains( VertexComponent::Weights ) )  memcpy( &vertex.weights,   src + vertex_layout_.OffsetOf( VertexComponent::Weights ),  sizeof( float   ) * 4 );

	return vertex;
}

FaceRange GeometryData::GetFaces( void ) const
{
	return FaceRange( face_data_.data(), ( face_data_.size() / ( 3 * index_size_ ) ), index_size_ );
}

uint8_t GeometryData::EvalIndexSize( size_t vertex_count )
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

IndexFormat GeometryData::EvalIndexFormat( void )
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
