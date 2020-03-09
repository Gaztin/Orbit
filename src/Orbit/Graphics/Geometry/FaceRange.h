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
#include "Orbit/Graphics/Geometry/Face.h"

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS FaceRange
{
	ORB_DISABLE_COPY( FaceRange );

public:

	struct Iterator
	{
		Iterator& operator++( void )                        { ++index; return *this; }
		Face      operator* ( void )                  const { return range->At( index ); }
		bool      operator!=( const Iterator& other ) const { return ( ( range != other.range ) || ( index != other.index ) ); }

		const FaceRange* range;

		size_t           index;
	};

public:

	explicit FaceRange( const uint8_t* ptr, size_t count, uint8_t index_size );

public:

	Face At( size_t index ) const;

public:

	Iterator begin( void ) const;
	Iterator end  ( void ) const;

private:

	const uint8_t* ptr_;

	size_t         count_;

	uint8_t        index_size_;

};

ORB_NAMESPACE_END
