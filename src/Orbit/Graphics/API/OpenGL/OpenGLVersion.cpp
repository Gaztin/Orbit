/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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

#include "OpenGLVersion.h"

#if defined( ORB_HAS_OPENGL )
#  include <cctype>
#  include <cstdio>
#  include <cstring>

ORB_NAMESPACE_BEGIN

OpenGLVersion::OpenGLVersion( void )
	: m_embedded { 0 }
	, m_major    { 0 }
	, m_minor    { 0 }
{
}

void OpenGLVersion::Init( void )
{
	const char* string                     = reinterpret_cast< const char* >( glGetString( GL_VERSION ) );
	auto        [ major, minor, embedded ] = TraverseVersionString( string );

	m_major    = static_cast< uint8_t >( major );
	m_minor    = static_cast< uint8_t >( minor );
	m_embedded = static_cast< uint8_t >( embedded );
}

bool OpenGLVersion::RequireGL( uint8_t major, uint8_t minor ) const
{
	if( m_embedded )
		return false;

	if( major > m_major )
		return false;

	if( minor > m_minor )
		return false;

	return true;
}

bool OpenGLVersion::RequireGLES( uint8_t major, uint8_t minor ) const
{
	if( !m_embedded )
		return false;

	if( major > m_major )
		return false;

	if( minor > m_minor )
		return false;

	return true;
}

std::array< uint32_t, 3 > OpenGLVersion::TraverseVersionString( const char* string )
{
	std::array< uint32_t, 3 > results { };

	if( std::sscanf( string, "%u.%u", &results[ 0 ], &results[ 1 ] ) > 0 )
		return results;

	if( std::strncmp( string, "OpenGL", 6 ) != 0 )
		return { };

	string += 6;

	if( std::strncmp( string, " ES", 3 ) != 0 )
		return { };

	string += 3;

	/* Embedded flag */
	results[ 2 ] = 1;

	if( std::sscanf( string + 1, "%u.%u", &results[ 0 ], &results[ 1 ] ) > 0 )
		return results;

	if( std::strncmp( string, "-CM", 3 ) != 0 )
		return { };

	string += 3;

	if( std::sscanf( string + 1, "%u.%u", &results[ 0 ], &results[ 1 ] ) > 0 )
		return results;

	return { };
}

ORB_NAMESPACE_END

#endif
