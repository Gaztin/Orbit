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
#include "Orbit/Graphics/Buffer/ConstantBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Geometry/GeometryData.h"
#include "Orbit/Graphics/Shader/Shader.h"
#include "Orbit/Math/Vector3.h"

#include <vector>

ORB_NAMESPACE_BEGIN

class IRenderer;
class Matrix4;

class ORB_API_GRAPHICS DebugManager : public Singleton< DebugManager >
{
public:

	using LineSegmentVector = std::vector< std::pair< Vector3, Vector3 > >;
	using SphereVector      = std::vector< Vector3 >;

public:

	DebugManager( void );

public:

	void PushLineSegment( Vector3 start, Vector3 end );
	void PushSphere     ( Vector3 center );
	void Render         ( IRenderer& renderer, const Matrix4& view_projection );
	void Flush          ( void );

private:

	Shader            shader_;

	LineSegmentVector lines_;   // TODO: Use @LineSegment
	SphereVector      spheres_; // TODO: Use @Sphere

	VertexBuffer      lines_vertex_buffer_;
	VertexBuffer      spheres_vertex_buffer_;

	ConstantBuffer    constant_buffer_;

	GeometryData      sphere_geometry_;

};

ORB_NAMESPACE_END
