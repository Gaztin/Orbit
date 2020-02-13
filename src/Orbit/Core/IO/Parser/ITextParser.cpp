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

ORB_NAMESPACE_BEGIN

ITextParser::ITextParser( ByteSpan data )
	: IParser( data )
{
}

void ITextParser::SkipWhitespace( void )
{
	while( m_offset < m_size && std::isspace( m_data[ m_offset ] ) )
		++m_offset;
}

std::string ITextParser::ReadAlphaNumeric( void )
{
	size_t off = m_offset;

	while( off < m_size && std::isalnum( m_data[ off ] ) )
		++off;

	const std::string alphanumeric( reinterpret_cast< const char* >( &m_data[ m_offset ] ), ( off - m_offset ) );

	m_offset = off;

	return alphanumeric;
}

std::string ITextParser::ReadPrintable( void )
{
	size_t off = m_offset;

	while( off < m_size && std::isprint( m_data[ off ] ) )
		++off;

	const std::string printable( reinterpret_cast< const char* >( &m_data[ m_offset ] ), ( off - m_offset ) );

	m_offset = off;

	return printable;
}

std::string ITextParser::ReadLiteral( void )
{
	if( m_data[ m_offset ] != '"' )
		return std::string();

	size_t off = ( ++m_offset );

	while( off < m_size && m_data[ off ] != '"' )
		++off;

	const std::string literal( reinterpret_cast< const char* >( &m_data[ m_offset ] ), off - m_offset );

	m_offset = off;

	if( m_offset < m_size )
		++m_offset;

	return literal;
}

std::string ITextParser::Peek( size_t length ) const
{
	if( ( m_offset + length ) > m_size )
		length = ( m_size - m_offset );

	return std::string( reinterpret_cast< const char* >( &m_data[ m_offset ] ), length );
}

bool ITextParser::ExpectString( std::string_view str )
{
	if( ( m_offset + str.length() ) > m_size )
		return false;

	const char* data = reinterpret_cast< const char* >( &m_data[ m_offset ] );

	if( std::strncmp( data, str.data(), str.length() ) == 0 )
	{
		m_offset += str.length();
		return true;
	}

	return false;
}

ORB_NAMESPACE_END
