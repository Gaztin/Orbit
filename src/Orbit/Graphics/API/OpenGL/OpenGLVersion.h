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
#include "Orbit/Graphics/Graphics.h"

#include <array>

#if( ORB_HAS_OPENGL )

/* 'major' and 'minor' may already be defined. See https://stackoverflow.com/a/22253389 */
#ifdef major
#  pragma push_macro( "major" )
#  undef major
#  define UNDEFINED_major
#endif // major
#ifdef minor
#  pragma push_macro( "minor" )
#  undef minor
#  define UNDEFINED_minor
#endif // minor

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS OpenGLVersion
{
public:

	OpenGLVersion( void );

public:

	void Init( void );

public:

	bool    RequireGL  ( uint8_t major, uint8_t minor = 0 ) const;
	bool    RequireGLES( uint8_t major, uint8_t minor = 0 ) const;

public:

	bool    IsEmbedded ( void ) const { return ( embedded_ != 0 ); }
	uint8_t GetMajor   ( void ) const { return major_; }
	uint8_t GetMinor   ( void ) const { return minor_; }

private:

	static std::array< uint32_t, 3 > TraverseVersionString( const char* string );

private:

	uint8_t embedded_ : 1;
	uint8_t major_    : 3;
	uint8_t minor_    : 4;

};

ORB_NAMESPACE_END

#if defined( UNDEFINED_minor )
#  pragma pop_macro( "minor" )
#  undef UNDEFINED_minor
#endif // UNDEFINED_minor
#if defined( UNDEFINED_major )
#  pragma pop_macro( "major" )
#  undef UNDEFINED_major
#endif // UNDEFINED_major

#endif // ORB_HAS_OPENGL
