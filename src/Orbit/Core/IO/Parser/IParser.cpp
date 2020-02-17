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

#include "IParser.h"

#include <cstring>

ORB_NAMESPACE_BEGIN

IParser::IParser( ByteSpan data )
	: data_  { data.Copy() }
	, size_  { data.Size() }
	, offset_{ 0 }
	, good_  { false }
{
}

void IParser::Skip( size_t size )
{
	if( ( offset_ + size ) < size_ ) offset_ += size;
	else                             offset_  = size_;
}

void IParser::ReadBytes( void* dst, size_t count )
{
	if( ( offset_ + count ) < size_ )
	{
		std::memcpy( dst, &data_[ offset_ ], count );
		offset_ += count;
	}
	else
	{
		std::memcpy( dst, &data_[ offset_ ], ( offset_ - size_ ) );
		offset_ = size_;
	}
}

bool IParser::IsEOF( void ) const
{
	return ( offset_ == size_ );
}

ORB_NAMESPACE_END
