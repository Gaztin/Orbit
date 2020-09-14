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

#include "XMLFile.h"

#include <cctype>

ORB_NAMESPACE_BEGIN

XMLFile::XMLFile( ByteSpan data )
{
	Init( data.Size() );

	const char* src = reinterpret_cast< const char* >( data.Ptr() );

	if( !ExpectString( src, R"(<?xml version="1.0" encoding="utf-8"?>)" ) )
		return;

	SkipWhitespace( src );

	while( !IsEOF() )
	{
		if( !ParseElement( src, &root_element_ ) )
			return;
	}
}

std::string XMLFile::ReadName( const char* src )
{
	// Search for digits, letters, periods, hyphens, underscores and colons
	constexpr std::string_view valid_characters          = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.-_:";
	const std::string_view     characters_to_be_searched = std::string_view( src + current_offset_, total_size_ - current_offset_ );
	const size_t               end_of_name               = characters_to_be_searched.find_first_not_of( valid_characters );
	const std::string_view     name                      = characters_to_be_searched.substr( 0, end_of_name );

	Skip( end_of_name );

	return std::string( name );
}

std::string XMLFile::ReadContent( const char* src )
{
	// Search for left-hand angular brackets
	const std::string_view characters_to_be_searched = std::string_view( src + current_offset_, total_size_ - current_offset_ );
	const size_t           end_of_content            = characters_to_be_searched.find_first_of( '<' );
	const std::string_view content                   = characters_to_be_searched.substr( 0, end_of_content );

	Skip( end_of_content );

	return std::string( content );
}

bool XMLFile::ParseElement( const char* src, XMLElement* parent )
{
	XMLElement element;

	if( !ExpectString( src, "<" ) )
		return false;

	element.name = ReadName( src );
	SkipWhitespace( src );

	// Attributes
	while( Peek( src, 1 ) != ">" && Peek( src, 2 ) != "/>" )
	{
		XMLAttribute attribute;
		attribute.name = ReadName( src );

		if( !ExpectString( src, "=" ) )
			return false;

		attribute.value = ReadCapturedStringLiteral( src );
		element.attributes.push_back( attribute );

		SkipWhitespace( src );
	}

	if( ExpectString( src, ">" ) )
	{
		SkipWhitespace( src );

		/* Does element have character data instead of child elements? */
		if( Peek( src, 1 ) != "<" )
		{
			element.content = ReadContent( src );

			SkipWhitespace( src );
			if( !ExpectString( src, "</" + element.name + ">" ) )
				return false;
		}
		else
		{
			while( !ExpectString( src, "</" + element.name + ">" ) )
			{
				if( !ParseElement( src, &element ) )
					return false;
			}
		}
	}
	else if( !ExpectString( src, "/>" ) )
	{
		return false;
	}

	parent->children.push_back( element );

	SkipWhitespace( src );

	return true;
}

ORB_NAMESPACE_END
