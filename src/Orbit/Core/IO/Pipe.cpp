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

#include "Pipe.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#else // ORB_OS_WINDOWS
#  include <unistd.h>
#endif // !ORB_OS_WINDOWS

ORB_NAMESPACE_BEGIN

Pipe::Pipe( void )
	: handle_read_ { invalid_file_handle }
	, handle_write_{ invalid_file_handle }
{

#if defined( ORB_OS_WINDOWS )

	CreatePipe( &handle_read_, &handle_write_, NULL, 0 );

#else // ORB_OS_WINDOWS

	int fds[ 2 ];

	if( pipe( fds ) == 0 )
	{
		handle_read_  = fds[ 0 ];
		handle_write_ = fds[ 1 ];
	}

#endif // !ORB_OS_WINDOWS

}

Pipe::~Pipe( void )
{

#if defined( ORB_OS_WINDOWS )

	CloseHandle( handle_write_ );
	CloseHandle( handle_read_ );

#else // ORB_OS_WINDOWS

	close( handle_write_ );
	close( handle_read_ );

#endif // !ORB_OS_WINDOWS

}

size_t Pipe::Read( void* dst, size_t size ) const
{

#if defined( ORB_OS_WINDOWS )

	if( DWORD bytes_read; ReadFile( handle_read_, dst, static_cast< DWORD >( size ), &bytes_read, NULL ) )
		return static_cast< size_t >( bytes_read );

#else // ORB_OS_WINDOWS

	if( ssize_t bytes_read = read( handle_read_, dst, size ); bytes_read >= 0 )
		return static_cast< size_t >( bytes_read );

#endif // !ORB_OS_WINDOWS

	return 0;
}

size_t Pipe::Write( const void* src, size_t size ) const
{

#if defined( ORB_OS_WINDOWS )

	if( DWORD bytes_written; WriteFile( handle_write_, src, static_cast< DWORD >( size ), &bytes_written, NULL ) )
		return static_cast< size_t >( bytes_written );

#else // ORB_OS_WINDOWS

	if( ssize_t bytes_written = write( handle_write_, src, size ); bytes_written >= 0 )
		return static_cast< size_t >( bytes_written );

#endif // !ORB_OS_WINDOWS

	return 0;
}

ORB_NAMESPACE_END
