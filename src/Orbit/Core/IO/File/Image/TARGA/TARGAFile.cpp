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

#include "TARGAFile.h"

#include <algorithm>

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

enum class PacketType : uint8_t
{
	Raw       = 0,
	RunLength = 1,
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

TARGAFile::TARGAFile( ByteSpan data )
{
	Init( data.Size() );

	Header header;

	ReadBytes( data.Ptr(), &header.id_length, 1 );
	ReadBytes( data.Ptr(), &header.color_map_type, 1 );
	ReadBytes( data.Ptr(), &header.image_type, 1 );
	ReadBytes( data.Ptr(), &header.color_map_specification.first_entry_index, 2 );
	ReadBytes( data.Ptr(), &header.color_map_specification.color_map_length, 2 );
	ReadBytes( data.Ptr(), &header.color_map_specification.color_map_entry_size, 1 );
	ReadBytes( data.Ptr(), &header.image_specification.x, 2 );
	ReadBytes( data.Ptr(), &header.image_specification.y, 2 );
	ReadBytes( data.Ptr(), &header.image_specification.width, 2 );
	ReadBytes( data.Ptr(), &header.image_specification.height, 2 );
	ReadBytes( data.Ptr(), &header.image_specification.depth, 1 );
	ReadBytes( data.Ptr(), &header.image_specification.descriptor, 1 );

	if( ( header.image_specification.width == 0 ) || ( header.image_specification.height == 0 ) )
		return;

	if( header.id_length > 0 )
		Skip( header.id_length );

	if( header.color_map_type != 0 )
	{
		for( uint16_t i = 0; i < header.color_map_specification.color_map_length ; ++i )
			Skip( header.color_map_specification.color_map_entry_size );
	}

//////////////////////////////////////////////////////////////////////////

	const size_t pixel_count = ( header.image_specification.width * header.image_specification.height );

	bytes_per_pixel_ = ( ( header.image_specification.depth + 7 ) / 8 );

	switch( header.image_type )
	{
		case ImageType::UncompressedTrueColor:
		{
			image_data_ = std::make_unique< uint32_t[] >( pixel_count );

			for( size_t i = 0; i < pixel_count; ++i )
				image_data_[ i ] = ReadTrueColor( data.Ptr() );

		} break;

		case ImageType::RunLengthEncodedTrueColor:
		{
			image_data_ = std::make_unique< uint32_t[] >( pixel_count );

			for( size_t pixels_read = 0; pixels_read < pixel_count; )
				pixels_read += ReadNextRLEPacket( data.Ptr(), &image_data_[ pixels_read ] );

		} break;

		default:
		{
			return;
		}
	}

	width_  = header.image_specification.width;
	height_ = header.image_specification.height;
}

uint32_t TARGAFile::ReadTrueColor( const void* src )
{
	uint32_t argb = 0xFF000000;
	ReadBytes( src, &argb, bytes_per_pixel_ );

	const uint32_t abgr = ( ( argb & 0xFF00FF00 ) | ( ( argb & 0x00FF0000 ) >> 16 ) | ( ( argb & 0x000000FF ) << 16 ) );
	return abgr;
}

size_t TARGAFile::ReadNextRLEPacket( const void* src, uint32_t* dst )
{
	uint8_t repetition_count_and_packet_type = 0;
	ReadBytes( src, &repetition_count_and_packet_type, 1 );

	const PacketType packet_type      = static_cast< PacketType >( ( repetition_count_and_packet_type & 0x80 ) >> 7 );
	const uint8_t    repetition_count = ( ( repetition_count_and_packet_type & 0x7f ) + 1 );

	switch( packet_type )
	{
		case PacketType::Raw:
		{
			for( size_t i = 0; i < repetition_count; ++i )
				dst[ i ] = ReadTrueColor( src );

		} break;

		case PacketType::RunLength:
		{
			const uint32_t pixel_value = ReadTrueColor( src );

			for( size_t i = 0; i < repetition_count; ++i )
				dst[ i ] = pixel_value;

		} break;
	}

	return repetition_count;
}

ORB_NAMESPACE_END
