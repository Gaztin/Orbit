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
#include "Orbit/Core/Utility/Span.h"

#include <memory>
#include <string_view>

ORB_NAMESPACE_BEGIN

/** Base class for file format parsers.
 * Does not store the data by default. This is by design in case the output can be processed in chunks rather than
 * needing to wait until the entire file has been parsed.
 */
class ORB_API_CORE BinaryFile
{
public:

	         BinaryFile( void ) = default;
	virtual ~BinaryFile( void ) = default;

protected:

	/** Initializes the metadata */
	void Init( size_t total_size );

	/** Skips forward @size amount of bytes */
	void Skip( size_t size );

	/** Reads @size amount of bytes from @src and writes them to @dst */
	void ReadBytes( const void* src, void* dst, size_t size );

	/** Returns whether or not the file has reached the end */
	bool IsEOF( void ) const;

protected:

	size_t total_size_     = 0;
	size_t current_offset_ = 0;

};

ORB_NAMESPACE_END
