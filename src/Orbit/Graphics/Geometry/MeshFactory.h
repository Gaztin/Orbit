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
#include "Orbit/Core/Shape/IShape.h"
#include "Orbit/Core/Utility/Singleton.h"
#include "Orbit/Graphics/Graphics.h"

#include <memory>
#include <string_view>
#include <vector>

ORB_NAMESPACE_BEGIN

class  Geometry;
class  Mesh;
class  VertexLayout;

class ORB_API_GRAPHICS MeshFactory final : public Singleton< MeshFactory >
{
public:

	enum class DetailLevel
	{
		Low,
		Medium,
		High,
	};

public:

	Geometry CreateGeometryFromShape( ShapeType shape_type, const VertexLayout& vertex_layout, DetailLevel detail_level = DetailLevel::Medium ) const;
	Mesh     CreateMeshFromShape    ( const IShape& shape, const VertexLayout& vertex_layout, DetailLevel detail_level = DetailLevel::Medium ) const;

private:

	std::string_view EvalShapeName     ( ShapeType type ) const;
	void             GenerateCubeData  ( Geometry& geometry ) const;
	void             GenerateSphereData( Geometry& geometry, DetailLevel detail_level ) const;

};

ORB_NAMESPACE_END
