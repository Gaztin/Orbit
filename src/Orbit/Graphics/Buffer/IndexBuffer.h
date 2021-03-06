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
#include "Orbit/Graphics/Private/IndexBufferDetails.h"

#include <initializer_list>
#include <type_traits>

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS IndexBuffer
{
public:

	IndexBuffer( IndexFormat fmt, const void* data, size_t count );

	template< typename T,
		typename = typename std::enable_if_t< is_index_format_v< T > > >
	IndexBuffer( std::initializer_list< T > indices )
		: IndexBuffer( index_format_v< T >, indices.begin(), indices.size() )
	{
	}

	~IndexBuffer( void );

public:

	void Bind( void );

public:

	size_t GetSize( void ) const;

public:

	Private::IndexBufferDetails&       GetDetails( void )       { return details_; }
	const Private::IndexBufferDetails& GetDetails( void ) const { return details_; }
	IndexFormat                        GetFormat ( void ) const { return format_; }
	size_t                             GetCount  ( void ) const { return count_; }

private:

	Private::IndexBufferDetails details_;
	IndexFormat                 format_;
	size_t                      count_;

};

ORB_NAMESPACE_END
