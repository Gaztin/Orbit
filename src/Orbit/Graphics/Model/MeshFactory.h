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
#include "Orbit/Core/Utility/Singleton.h"
#include "Orbit/Graphics/Graphics.h"

#include <memory>

ORB_NAMESPACE_BEGIN

class  CubeShape;
class  IShape;
class  VertexLayout;
struct Mesh;

class ORB_API_GRAPHICS MeshFactory final : public Singleton< MeshFactory >
{
public:

	Mesh CreateMeshFromShape( const IShape& shape, const VertexLayout& vertex_layout ) const;

private:

	void GenerateCubeData  ( uint8_t* vertex_data, uint16_t* index_data, const VertexLayout& vertex_layout ) const;
	void GenerateSphereData( uint8_t* vertex_data, uint16_t* index_data, const VertexLayout& vertex_layout ) const;
	void GenerateNormals   ( uint8_t* vertex_data, const uint16_t* index_data, size_t face_count, const VertexLayout& vertex_layout ) const;

};

ORB_NAMESPACE_END
