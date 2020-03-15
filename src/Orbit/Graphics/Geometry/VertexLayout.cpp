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

#include "VertexLayout.h"

#include <cassert>
#include <limits>

ORB_NAMESPACE_BEGIN

constexpr size_t invalid_offset = std::numeric_limits< size_t >::max();

static size_t DataCountOf( VertexComponent component )
{
	switch( component )
	{
		default: { assert( false ); return 0; }

		case VertexComponent::Position: return 4;
		case VertexComponent::Normal:   return 3;
		case VertexComponent::Color:    return 4;
		case VertexComponent::TexCoord: return 2;
		case VertexComponent::JointIDs: return 4;
		case VertexComponent::Weights:  return 4;
	}
}

static PrimitiveDataType DataTypeOf( VertexComponent component )
{
	switch( component )
	{
		default: { assert( false ); return static_cast< PrimitiveDataType >( ~0 ); }

		case Orbit::VertexComponent::Position:
		case Orbit::VertexComponent::Normal:
		case Orbit::VertexComponent::Color:
		case Orbit::VertexComponent::TexCoord:
		case Orbit::VertexComponent::Weights:
			return PrimitiveDataType::Float;

		case Orbit::VertexComponent::JointIDs:
			return PrimitiveDataType::Int;
	}
}

static size_t SizeOf( VertexComponent component )
{
	return ( 4 * DataCountOf( component ) );
}

size_t IndexedVertexComponent::GetSize( void ) const
{
	return SizeOf( type );
}

size_t IndexedVertexComponent::GetDataCount( void ) const
{
	return DataCountOf( type );
}

PrimitiveDataType IndexedVertexComponent::GetDataType( void ) const
{
	return DataTypeOf( type );
}

bool VertexComponentIterator::operator!=( const VertexComponentIterator& other ) const
{
	/* Trying to compare iterator from another layout */
	assert( layout == other.layout );

	return ( indexed_component.index != other.indexed_component.index );
}

IndexedVertexComponent VertexComponentIterator::operator*( void ) const
{
	return indexed_component;
}

VertexComponentIterator& VertexComponentIterator::operator++( void )
{
	++indexed_component.index;

	if( indexed_component.index < layout->components_.size() )
		indexed_component.type = layout->components_[ indexed_component.index ];

	return *this;
}

VertexLayout::VertexLayout( VertexLayout&& other )
	: components_( std::move( other.components_ ) )
{
}

VertexLayout::VertexLayout( std::initializer_list< VertexComponent > components )
	: components_{ components }
{
}

void VertexLayout::Add( VertexComponent component )
{
	components_.push_back( component );
}

size_t VertexLayout::GetStride( void ) const
{
	size_t stride = 0;

	for( VertexComponent vc : components_ )
		stride += SizeOf( vc );

	return stride;
}

size_t VertexLayout::GetCount( void ) const
{
	return components_.size();
}

size_t VertexLayout::OffsetOf( VertexComponent component ) const
{
	size_t offset = 0;

	for( VertexComponent vc : components_ )
	{
		if( vc == component )
			return offset;

		offset += SizeOf( vc );
	}

	return invalid_offset;
}

bool VertexLayout::Contains( VertexComponent component ) const
{
	for( VertexComponent vc : components_ )
	{
		if( vc == component )
			return true;
	}

	return false;
}

VertexComponentIterator VertexLayout::begin( void ) const
{
	if( !components_.empty() )
	{
		IndexedVertexComponent indexed_component{ components_.front(), 0 };

		return { this, indexed_component };
	}

	return end();
}

VertexComponentIterator VertexLayout::end( void ) const
{
	IndexedVertexComponent indexed_component{ std::numeric_limits< VertexComponent >::max(), components_.size() };

	return { this, indexed_component };
}

VertexLayout& VertexLayout::operator=( VertexLayout&& other )
{
	components_ = std::move( other.components_ );

	return *this;
}

ORB_NAMESPACE_END
