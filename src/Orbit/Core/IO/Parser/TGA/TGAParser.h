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
#include "Orbit/Core/IO/Parser/IParser.h"

#include <cstdint>
#include <memory>
#include <vector>

ORB_NAMESPACE_BEGIN

class ORB_API_CORE TGAParser : public IParser
{
public:

	explicit TGAParser( ByteSpan data );

public:

	const uint32_t* ImageData( void ) const { return image_data_.get(); }
	uint16_t        Width    ( void ) const { return width_; }
	uint16_t        Height   ( void ) const { return height_; }

private:

	uint32_t ReadTrueColor    ( void );
	size_t   ReadNextRLEPacket( uint32_t* dst );

private:

	std::unique_ptr< uint32_t[] > image_data_;

	size_t bytes_per_pixel_;

	uint16_t width_;
	uint16_t height_;

};

ORB_NAMESPACE_END
