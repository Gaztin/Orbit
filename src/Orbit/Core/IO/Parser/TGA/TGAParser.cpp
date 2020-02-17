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

#include "TGAParser.h"

#include <algorithm>
#include <intrin.h>

ORB_NAMESPACE_BEGIN

enum class ImageType : uint8_t
{
	NoImage                       = 0,
	UncompressedColorMapped       = 1,
	UncompressedTrueColor         = 2,
	UncompressedBlackAndWhite     = 3,
	RunLengthEncodedColorMapped   = 9,
	RunLengthEncodedTrueColor     = 10,
	RunLengthEncodedBlackAndWhite = 11,
};

struct ColorMapSpecification
{
	uint16_t first_entry_index;
	uint16_t color_map_length;
	uint8_t  color_map_entry_size;
};

struct ImageDescriptor
{
	uint8_t alpha_channel_bits : 4;
	uint8_t image_origin       : 2;
	uint8_t unused             : 2;
};

struct ImageSpecification
{
	uint16_t        x;
	uint16_t        y;
	uint16_t        width;
	uint16_t        height;
	uint8_t         depth;
	ImageDescriptor descriptor;
};

struct Header
{
	uint8_t               id_length;
	uint8_t               color_map_type;
	ImageType             image_type;
	ColorMapSpecification color_map_specification;
	ImageSpecification    image_specification;
};

TGAParser::TGAParser( ByteSpan data )
	: IParser( data )
{
	Header header;
	ReadBytes( &header.id_length, 1 );
	ReadBytes( &header.color_map_type, 1 );
	ReadBytes( &header.image_type, 1 );
	ReadBytes( &header.color_map_specification.first_entry_index, 2 );
	ReadBytes( &header.color_map_specification.color_map_length, 2 );
	ReadBytes( &header.color_map_specification.color_map_entry_size, 1 );
	ReadBytes( &header.image_specification.x, 2 );
	ReadBytes( &header.image_specification.y, 2 );
	ReadBytes( &header.image_specification.width, 2 );
	ReadBytes( &header.image_specification.height, 2 );
	ReadBytes( &header.image_specification.depth, 1 );
	ReadBytes( &header.image_specification.descriptor, 1 );

	if( header.id_length > 0 )
		Skip( header.id_length );

	if( header.color_map_type != 0 )
	{
		for( uint16_t i = 0; i < header.color_map_specification.color_map_length ; ++i )
			Skip( header.color_map_specification.color_map_entry_size );
	}

	switch( header.image_type )
	{
		case ImageType::UncompressedTrueColor:
		{
			const size_t bits_per_pixel = header.image_specification.depth;
			const size_t pixel_count    = ( header.image_specification.width * header.image_specification.height );

			image_data_.reset( new uint8_t[ pixel_count * 4 ] );

			for( size_t i = 0; i < pixel_count; ++i )
			{
				ReadBytes( &image_data_[ i * 4 + 0 ], ( bits_per_pixel / 8 ) );
				image_data_[ i * 4 + 3 ] = 255;
			}

		} break;
	}

	width_  = header.image_specification.width;
	height_ = header.image_specification.height;
	good_   = true;
}

ORB_NAMESPACE_END
