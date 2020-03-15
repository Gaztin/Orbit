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
#include "Orbit/Graphics/Graphics.h"

#include <vector>

ORB_NAMESPACE_BEGIN

enum class VertexComponent : uint8_t
{
	Position,
	Normal,
	Color,
	TexCoord,
	JointIDs,
	Weights,
};

struct IndexedVertexComponent
{
	size_t            GetSize     ( void ) const;
	size_t            GetDataCount( void ) const;
	PrimitiveDataType GetDataType ( void ) const;

	VertexComponent type;
	size_t          index;
};

class VertexLayout;

struct ORB_API_GRAPHICS VertexComponentIterator
{
	bool                     operator!=( const VertexComponentIterator& other ) const;
	IndexedVertexComponent   operator* ( void )                                 const;
	VertexComponentIterator& operator++( void );

	const VertexLayout*    layout;
	IndexedVertexComponent indexed_component;
};

class ORB_API_GRAPHICS VertexLayout
{
	friend struct VertexComponentIterator;

public:

	VertexLayout( void )                      = default;
	VertexLayout( const VertexLayout& other ) = default;
	VertexLayout( VertexLayout&& other );
	VertexLayout( std::initializer_list< VertexComponent > components );

public:

	void Add( VertexComponent component );

public:

	size_t GetStride( void )                      const;
	size_t GetCount ( void )                      const;
	size_t OffsetOf ( VertexComponent component ) const;
	bool   Contains ( VertexComponent component ) const;

public:

	VertexComponentIterator begin( void ) const;
	VertexComponentIterator end  ( void ) const;

public:

	VertexLayout& operator=( const VertexLayout& other ) = default;
	VertexLayout& operator=( VertexLayout&& other );

private:

	std::vector< VertexComponent > components_;

};

ORB_NAMESPACE_END
