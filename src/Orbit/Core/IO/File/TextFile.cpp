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

#include "TextFile.h"

#include <algorithm>
#include <cctype>
#include <cstring>

ORB_NAMESPACE_BEGIN

void TextFile::SkipWhitespace( const char* src )
{
	while( current_offset_ < total_size_ && std::isspace( src[ current_offset_ ] ) )
		++current_offset_;
}

bool TextFile::ExpectString( const char* src, std::string_view str )
{
	if( ( current_offset_ + str.length() ) > total_size_ )
		return false;

	// Compare characters
	if( std::strncmp( src + current_offset_, str.data(), str.length() ) == 0 )
	{
		current_offset_ += str.length();
		return true;
	}

	return false;
}

std::string TextFile::ReadAlphaNumeric( const char* src )
{
	// Find end of alphanumeric string
	size_t end = current_offset_;
	while( end < total_size_ && std::isalnum( src[ end ] ) )
		++end;

	// Construct string from alphanumeric characters
	const std::string string( src + current_offset_, end - current_offset_ );

	// Move offset to end of string
	current_offset_ = end;

	return string;
}

std::string TextFile::ReadPrintable( const char* src )
{
	// Find end of printable string
	size_t end = current_offset_;
	while( end < total_size_ && std::isprint( src[ end ] ) )
		++end;

	// Construct string from printable characters
	const std::string printable( src + current_offset_, end - current_offset_ );

	// Move offset to end of string
	current_offset_ = end;

	return printable;
}

std::string TextFile::ReadCapturedStringLiteral( const char* src )
{
	// Expect quotation mark
	if( !ExpectString( src, "\"" ) )
		return std::string();

	// Find matching quotation mark
	const size_t end = FindCharacter( src, '"' );

	// Construct captured string
	const std::string string( src + current_offset_, end );

	// Move offset to end of string
	current_offset_ += end;

	// Skip last quotation mark
	Skip( 1 );

	return string;
}

size_t TextFile::FindCharacter( const char* src, char c ) const
{
	const char* begin = src + current_offset_;
	const char* it    = begin;

	// Increment offset until character is uncovered
	for( ; *it != c; ++it );

	return it - begin;
}

std::string TextFile::Peek( const char* src, size_t length ) const
{
	// Clamp length so we don't read more characters than we have available
	length = std::min( length, current_offset_ - total_size_ );

	return std::string( src + current_offset_, length );
}

ORB_NAMESPACE_END
