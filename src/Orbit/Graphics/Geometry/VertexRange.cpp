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

#include "VertexRange.h"

#include "Orbit/Graphics/Geometry/Geometry.h"

ORB_NAMESPACE_BEGIN

VertexRange::Iterator& VertexRange::Iterator::operator++( void )
{
	++index;

	return *this;
}

Vertex VertexRange::Iterator::operator*( void ) const
{
	return range->geometry_->GetVertex( index );
}

bool VertexRange::Iterator::operator!=( const Iterator& other ) const
{
	return ( ( range != other.range ) || ( index != other.index ) );
}

VertexRange::VertexRange( const Geometry* geometry )
	: geometry_( geometry )
{
}

VertexRange::Iterator VertexRange::begin( void ) const
{
	return Iterator{ this, 0 };
}

VertexRange::Iterator VertexRange::end( void ) const
{
	return Iterator{ this, geometry_->GetVertexCount() };
}

ORB_NAMESPACE_END
