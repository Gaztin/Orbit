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
#include "Orbit/Core/Utility/Ref.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Geometry/VertexLayout.h"
#include "Orbit/Math/Matrix/Matrix4.h"

#include <memory>
#include <string_view>
#include <string>
#include <vector>

ORB_NAMESPACE_BEGIN

class Plane;

class ORB_API_GRAPHICS Mesh
{
	friend class Geometry;

public:

	explicit Mesh( std::string_view name );

public:

	std::vector< Mesh > Slice( const Plane& plane ) const;

public:

	std::string_view    GetName        ( void ) const { return name_; }
	Ref< VertexBuffer > GetVertexBuffer( void ) const { return vertex_buffer_ ? Ref( *vertex_buffer_ ) : nullptr; }
	Ref< IndexBuffer >  GetIndexBuffer ( void ) const { return index_buffer_  ? Ref( *index_buffer_  ) : nullptr; }

public:

	Matrix4 transform;

private:

	VertexLayout                    vertex_layout_;

	std::string                     name_;

	std::unique_ptr< VertexBuffer > vertex_buffer_;
	std::unique_ptr< IndexBuffer >  index_buffer_;
};

ORB_NAMESPACE_END
