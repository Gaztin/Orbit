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

#pragma once
#include "Orbit/Core/Utility/Span.h"
#include "Orbit/Graphics/Geometry/Face.h"
#include "Orbit/Graphics/Geometry/FaceRange.h"
#include "Orbit/Graphics/Geometry/Mesh.h"
#include "Orbit/Graphics/Geometry/Vertex.h"
#include "Orbit/Graphics/Geometry/VertexLayout.h"
#include "Orbit/Graphics/Geometry/VertexRange.h"

#include <vector>

ORB_NAMESPACE_BEGIN

struct Face;
struct Vertex;

class ORB_API_GRAPHICS Geometry
{
	ORB_DISABLE_COPY( Geometry );

public:

	explicit Geometry( const VertexLayout& vertex_layout );
	         Geometry( Geometry&& other );

public:

	void   SetFromData    ( ByteSpan vertex_data );
	void   SetFromData    ( ByteSpan vertex_data, ByteSpan face_data, IndexFormat index_format );
	void   Reserve        ( size_t vertex_count, size_t face_count );
	size_t AddFace        ( const Face& face );
	size_t AddVertex      ( const Vertex& vertex );
	void   SetVertex      ( size_t index, const Vertex& vertex );
	void   GenerateNormals( void );

public:

	VertexLayout GetVertexLayout( void )                  const { return vertex_layout_; }
	size_t       GetVertexCount ( void )                  const;
	size_t       GetFaceCount   ( void )                  const;
	Vertex       GetVertex      ( size_t index )          const;
	Face         GetFace        ( size_t index )          const;
	FaceRange    GetFaces       ( void )                  const;
	VertexRange  GetVertices    ( void )                  const;
	Mesh         ToMesh         ( std::string_view name ) const;

public:

	Geometry& operator=( Geometry&& other );

private:

	void UpgradeFaceData( uint8_t new_index_size );

private:

	uint8_t     EvalIndexSize ( size_t index_or_vertex_count ) const;
	IndexFormat GetIndexFormat( void )                         const;

private:

	VertexLayout           vertex_layout_;

	std::vector< uint8_t > vertex_data_;
	std::vector< uint8_t > face_data_;

	uint8_t                index_size_;

};

ORB_NAMESPACE_END
