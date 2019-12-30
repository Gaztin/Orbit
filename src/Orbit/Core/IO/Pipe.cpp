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

#include "Pipe.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#else
#  include <unistd.h>
#endif

ORB_NAMESPACE_BEGIN

Pipe::Pipe( void )
	: m_handle_read ( invalid_file_handle )
	, m_handle_write( invalid_file_handle )
{

#if defined( ORB_OS_WINDOWS )

	CreatePipe( &m_handle_read, &m_handle_write, NULL, 0 );

#else

	int msgpipe[ 2 ];

	if( pipe( msgpipe ) == 0 )
	{
		m_handle_read  = msgpipe[ 0 ];
		m_handle_write = msgpipe[ 1 ];
	}

#endif

}

Pipe::~Pipe( void )
{

#if defined( ORB_OS_WINDOWS )

	CloseHandle( m_handle_write );
	CloseHandle( m_handle_read );

#else

	close( m_handle_write );
	close( m_handle_read );

#endif

}

size_t Pipe::Read( void* dst, size_t size ) const
{

#if defined( ORB_OS_WINDOWS )

	if( DWORD bytes_read; ReadFile( m_handle_read, dst, static_cast< DWORD >( size ), &bytes_read, NULL ) )
	{
		return static_cast< size_t >( bytes_read );
	}

#else

	if( ssize_t bytes_read = read( m_handle_read, dst, size ); bytes_read >= 0 )
	{
		return static_cast< size_t >( bytes_read );
	}

#endif

	return 0;
}

size_t Pipe::Write( const void* src, size_t size ) const
{

#if defined( ORB_OS_WINDOWS )

	if( DWORD bytes_written; WriteFile( m_handle_write, src, static_cast< DWORD >( size ), &bytes_written, NULL ) )
	{
		return static_cast< size_t >( bytes_written );
	}

#else

	if( ssize_t bytes_written = write( m_handle_write, src, size ); bytes_written >= 0 )
	{
		return static_cast< size_t >( bytes_written );
	}

#endif

	return 0;
}

ORB_NAMESPACE_END
