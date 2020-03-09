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

#include "FaceRange.h"

ORB_NAMESPACE_BEGIN

FaceRange::FaceRange( const uint8_t* ptr, size_t count, uint8_t index_size )
	: ptr_       ( ptr )
	, count_     ( count )
	, index_size_( index_size )
{
}

Face FaceRange::At( size_t index ) const
{
	Face face{ };

	for( size_t i = 0; i < 3; ++i )
		memcpy( &face.indices[ i ], &ptr_[ ( index * 3 + i ) * index_size_ ], index_size_ );

	return face;
}

FaceRange::Iterator FaceRange::begin( void ) const
{
	return Iterator{ this, 0 };
}

FaceRange::Iterator FaceRange::end( void ) const
{
	return Iterator{ this, count_ };
}

ORB_NAMESPACE_END
