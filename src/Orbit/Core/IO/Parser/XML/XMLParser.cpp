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

#include "XMLParser.h"

#include <cctype>

ORB_NAMESPACE_BEGIN

XMLParser::XMLParser( ByteSpan data )
	: ITextParser( data )
{
	if( !ExpectString( R"(<?xml version="1.0" encoding="utf-8"?>)" ) )
		return;

	SkipWhitespace();

	while( !IsEOF() )
	{
		if( !ParseElement( &root_element_ ) )
			return;
	}

	good_ = true;
}

std::string XMLParser::ReadName( void )
{
	size_t off = offset_;

	while( off < size_ && ( std::isalnum( data_[ off ] ) || data_[ off ] == '_' ) )
		++off;

	const std::string name( reinterpret_cast< const char* >( &data_[ offset_ ] ), ( off - offset_ ) );

	offset_ = off;

	return name;
}

std::string XMLParser::ReadContent( void )
{
	size_t off = offset_;

	while( off < size_ && data_[ off ] != '<' && ( std::isprint( data_[ off ] ) || std::isspace( data_[ off ] ) ) )
		++off;

	const std::string name( reinterpret_cast< const char* >( &data_[ offset_ ] ), ( off - offset_ ) );

	offset_ = off;

	return name;
}

bool XMLParser::ParseElement( XMLElement* parent )
{
	XMLElement element;

	if( !ExpectString( "<" ) )
		return false;

	element.name = ReadName();
	SkipWhitespace();

	/* Attributes */
	while( Peek( 1 ) != ">" && Peek( 2 ) != "/>" )
	{
		XMLAttribute attribute;
		attribute.name = ReadName();

		if( !ExpectString( "=" ) )
			return false;

		attribute.value = ReadLiteral();
		element.attributes.push_back( attribute );

		SkipWhitespace();
	}

	if( ExpectString( ">" ) )
	{
		SkipWhitespace();

		/* Does element have character data instead of child elements? */
		if( Peek( 1 ) != "<" )
		{
			element.content = ReadContent();

			SkipWhitespace();
			if( !ExpectString( "</" + element.name + ">" ) )
				return false;
		}
		else
		{
			while( !ExpectString( "</" + element.name + ">" ) )
			{
				if( !ParseElement( &element ) )
					return false;
			}
		}
	}
	else if( !ExpectString( "/>" ) )
	{
		return false;
	}

	parent->children.push_back( element );

	SkipWhitespace();

	return true;
}

ORB_NAMESPACE_END
