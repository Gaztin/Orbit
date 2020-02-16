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

#include "ITextParser.h"

#include <algorithm>
#include <cctype>
#include <cstring>

ORB_NAMESPACE_BEGIN

ITextParser::ITextParser( ByteSpan data )
	: IParser( data )
{
}

void ITextParser::SkipWhitespace( void )
{
	while( offset_ < size_ && std::isspace( data_[ offset_ ] ) )
		++offset_;
}

std::string ITextParser::ReadAlphaNumeric( void )
{
	size_t off = offset_;

	while( off < size_ && std::isalnum( data_[ off ] ) )
		++off;

	const std::string alphanumeric( reinterpret_cast< const char* >( &data_[ offset_ ] ), ( off - offset_ ) );

	offset_ = off;

	return alphanumeric;
}

std::string ITextParser::ReadPrintable( void )
{
	size_t off = offset_;

	while( off < size_ && std::isprint( data_[ off ] ) )
		++off;

	const std::string printable( reinterpret_cast< const char* >( &data_[ offset_ ] ), ( off - offset_ ) );

	offset_ = off;

	return printable;
}

std::string ITextParser::ReadLiteral( void )
{
	if( data_[ offset_ ] != '"' )
		return std::string();

	size_t off = ( ++offset_ );

	while( off < size_ && data_[ off ] != '"' )
		++off;

	const std::string literal( reinterpret_cast< const char* >( &data_[ offset_ ] ), off - offset_ );

	offset_ = off;

	if( offset_ < size_ )
		++offset_;

	return literal;
}

std::string ITextParser::Peek( size_t length ) const
{
	if( ( offset_ + length ) > size_ )
		length = ( size_ - offset_ );

	return std::string( reinterpret_cast< const char* >( &data_[ offset_ ] ), length );
}

bool ITextParser::ExpectString( std::string_view str )
{
	if( ( offset_ + str.length() ) > size_ )
		return false;

	const char* data = reinterpret_cast< const char* >( &data_[ offset_ ] );

	if( std::strncmp( data, str.data(), str.length() ) == 0 )
	{
		offset_ += str.length();
		return true;
	}

	return false;
}

ORB_NAMESPACE_END
