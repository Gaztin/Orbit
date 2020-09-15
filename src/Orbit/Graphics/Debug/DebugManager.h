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
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Geometry/Geometry.h"
#include "Orbit/Graphics/Shader/Shader.h"
#include "Orbit/Math/Geometry/LineSegment.h"
#include "Orbit/Math/Vector/Vector3.h"

#include <chrono>
#include <vector>

ORB_NAMESPACE_BEGIN

class IRenderer;
class Matrix4;

class ORB_API_GRAPHICS DebugManager : public Singleton< DebugManager >
{
public:

	using Clock = std::chrono::steady_clock;

	struct DebugObjectBase
	{
		Clock::time_point birth;
		Clock::time_point death;
		RGBA              color;
	};

	struct DebugLineSegment : DebugObjectBase
	{
		LineSegment line_segment;
	};

	struct DebugSphere : DebugObjectBase
	{
		Vector3 position;
	};

	using LineSegmentVector = std::vector< DebugLineSegment >;
	using SphereVector      = std::vector< DebugSphere >;

public:

	DebugManager( void );

public:

	void PushLineSegment( const LineSegment& line_segment, RGBA color, double duration = 0.0 );
	void PushSphere     ( Vector3 center, RGBA color, double duration = 0.0 );
	void Render         ( IRenderer& renderer, const Matrix4& view_projection );
	void Flush          ( void );

private:

	float CalcAlphaForObject( const DebugObjectBase& object, Clock::time_point now );

private:

	Shader            shader_;
	LineSegmentVector line_segments_;
	SphereVector      spheres_;
	VertexBuffer      lines_vertex_buffer_;
	VertexBuffer      spheres_vertex_buffer_;
	Geometry          sphere_geometry_;

};

ORB_NAMESPACE_END
