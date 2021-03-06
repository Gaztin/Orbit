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
#include "Orbit/Core/Core.h"

#include <initializer_list>

ORB_NAMESPACE_BEGIN

template< typename ValueType >
class BinarySearchTree
{
	ORB_DISABLE_COPY( BinarySearchTree );

public:

	constexpr BinarySearchTree( void )
		: root_( nullptr )
	{
	}

	constexpr BinarySearchTree( BinarySearchTree&& other )
		: root_( other.root_ )
	{
		other.root_ = nullptr;
	}

	BinarySearchTree( std::initializer_list< ValueType > values )
	{
		for( const ValueType& value : values )
			Insert( value );
	}

public:

	// #TODO: Implement constexpr version
	void Insert( const ValueType& value )
	{
		Node** insert_at = &root_;

		while( *insert_at )
		{
			if( value < ( *insert_at )->value ) insert_at = &( ( *insert_at )->left );
			else                                insert_at = &( ( *insert_at )->right );
		}

		( *insert_at )        = new Node;
		( *insert_at )->value = value;
	}

	constexpr const ValueType* Search( const ValueType& value ) const
	{
		if( const Node* node = SearchNode( value ); node != nullptr )
			return &node->value;

		return nullptr;
	}

private:

	struct Node
	{
		Node* left  = nullptr;
		Node* right = nullptr;

		ValueType value;
	};

private:

	constexpr const Node* SearchNode( const ValueType& value ) const
	{
		const Node* current_node = root_;

		while( current_node )
		{
			/**/ if( value == current_node->value ) return current_node;
			else if( value < current_node->value )  current_node = current_node->left;
			else                                    current_node = current_node->right;
		}

		return nullptr;
	}

private:

	Node* root_;

};

ORB_NAMESPACE_END
