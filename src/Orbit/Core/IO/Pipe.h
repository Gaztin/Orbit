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

#pragma once
#include <cstddef>
#include <memory>
#include <type_traits>

#include "Orbit/Core/IO/IO.h"

ORB_NAMESPACE_BEGIN

class ORB_API_CORE Pipe
{
public:

	 Pipe( void );
	~Pipe( void );

public:

	size_t Read ( void* dst, size_t size ) const;
	size_t Write( const void* src, size_t size ) const;

public:

	NativeFileHandle GetFileHandleRead ( void ) const { return m_handle_read; }
	NativeFileHandle GetFileHandleWrite( void ) const { return m_handle_write; }

public:

	template< typename T,
	    typename = typename std::enable_if_t< std::is_trivially_copyable_v< T > && std::is_default_constructible_v< T > > >
	T Read( void )
	{
		if( T val; Read( std::addressof( val ), sizeof( T ) ) == sizeof( T ) )
		{
			return val;
		}

		return T { };
	}

	template< typename T,
		typename = typename std::enable_if_t< std::is_trivially_copyable_v< T > > >
	void Write( const T& val )
	{
		Write( std::addressof( val ), sizeof( T ) );
	}

private:

	NativeFileHandle m_handle_read;
	NativeFileHandle m_handle_write;

};

ORB_NAMESPACE_END
