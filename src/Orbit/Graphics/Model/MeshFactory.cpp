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

#include "MeshFactory.h"

#include "Orbit/Core/Shape/IShape.h"
#include "Orbit/Core/Utility/Selector.h"
#include "Orbit/Graphics/Model/Mesh.h"
#include "Orbit/Graphics/Shader/VertexLayout.h"

ORB_NAMESPACE_BEGIN

constexpr Selector< ShapeType, size_t > selector_vertex_count
{
	{ ShapeType::Cube,   8 },
	{ ShapeType::Sphere, 512 },
};

constexpr Selector< ShapeType, size_t > selector_face_count
{
	{ ShapeType::Cube,   36 },
	{ ShapeType::Sphere, 4096 },
};

static size_t ElectVertexCount( ShapeType type )
{
	switch( type )
	{
		case ShapeType::Cube: return 8;
	}
}

std::unique_ptr< Mesh > MeshFactory::CreateMeshFromShape( const IShape& shape, const VertexLayout& vertex_layout ) const
{
	auto mesh           = std::make_unique< Mesh >();
	mesh->vertex_buffer = std::make_unique< VertexBuffer >( nullptr, selector_vertex_count[ shape.GetType() ], vertex_layout.GetStride() );
	mesh->index_buffer  = std::make_unique< IndexBuffer >( IndexFormat::Word, nullptr, ( selector_face_count[ shape.GetType() ] * 3 ) );

	// #TODO: Fill buffers

	return mesh;
}

ORB_NAMESPACE_END
