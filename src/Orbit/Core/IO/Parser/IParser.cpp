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

#include "IParser.h"

#include <cstring>

ORB_NAMESPACE_BEGIN

IParser::IParser( ByteSpan data )
	: m_data  ( data.Copy() )
	, m_size  ( data.GetSize() )
	, m_offset( 0 )
	, m_good  ( false )
{
}

bool IParser::ExpectString( std::string_view str )
{
	if( ( m_offset + str.length() ) > m_size )
		return false;

	return ( std::strncmp( reinterpret_cast< const char* >( &m_data[ m_offset ] ), str.data(), str.length() ) == 0 );
}

ORB_NAMESPACE_END
