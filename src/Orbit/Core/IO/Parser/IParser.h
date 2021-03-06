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

class ORB_API_CORE IParser
{
public:

	explicit IParser( ByteSpan data );
	virtual ~IParser( void ) = default;

public:

	bool IsGood( void ) const { return good_; }

protected:

	void Skip     ( size_t size );
	void ReadBytes( void* dst, size_t count );

protected:

	bool IsEOF( void ) const;

protected:

	std::unique_ptr< uint8_t[] > data_;

	size_t                       size_;
	size_t                       offset_;

	bool                         good_;

};

ORB_NAMESPACE_END
