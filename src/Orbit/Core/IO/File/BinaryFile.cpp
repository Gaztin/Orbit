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

#include "BinaryFile.h"

#include <cstring>

ORB_NAMESPACE_BEGIN

void BinaryFile::Init( size_t total_size )
{
	total_size_     = total_size;
	current_offset_ = 0;
}

void BinaryFile::Skip( size_t size )
{
	// Increment offset, but clamp it so it doesn't exceed the total size
	current_offset_ = std::min( current_offset_ + size, total_size_ );
}

void BinaryFile::ReadBytes( const void* src, void* dst, size_t size )
{
	// Make sure we don't read more bytes than we have
	const size_t bytes_to_write = std::min( size, total_size_ - current_offset_ );

	// Copy bytes from src to dst
	std::memcpy( dst, static_cast< const uint8_t* >( src ) + current_offset_, bytes_to_write );

	// Increment offset
	current_offset_ += bytes_to_write;
}

bool BinaryFile::IsEOF( void ) const
{
	return ( current_offset_ == total_size_ );
}

ORB_NAMESPACE_END
