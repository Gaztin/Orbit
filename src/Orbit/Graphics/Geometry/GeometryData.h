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
#include "Orbit/Graphics/Geometry/Face.h"
#include "Orbit/Graphics/Geometry/FaceRange.h"
#include "Orbit/Graphics/Geometry/Mesh.h"
#include "Orbit/Graphics/Geometry/Vertex.h"
#include "Orbit/Graphics/Geometry/VertexLayout.h"

#include <vector>

ORB_NAMESPACE_BEGIN

struct Face;
struct Vertex;

class ORB_API_GRAPHICS GeometryData
{
public:

	explicit GeometryData( size_t max_vertex_count, const VertexLayout& vertex_layout );

public:

	void Reserve  ( size_t vertex_count, size_t face_count );
	void AddFace  ( const Face& face );
	void AddVertex( const Vertex& vertex );
	void SetVertex( size_t index, const Vertex& vertex );
	Mesh ToMesh   ( void );

public:

	Vertex    GetVertex( size_t index ) const;
	FaceRange GetFaces ( void )         const;

private:

	uint8_t     EvalIndexSize  ( size_t vertex_count );
	IndexFormat EvalIndexFormat( void );

private:

	VertexLayout           vertex_layout_;

	std::vector< uint8_t > vertex_data_;
	std::vector< uint8_t > face_data_;

	uint8_t                index_size_;

};

ORB_NAMESPACE_END
