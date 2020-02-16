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

#include "Log.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#elif defined( ORB_OS_ANDROID ) // ORB_OS_WINDOWS
#  include <android/log.h>
#endif // ORB_OS_ANDROID

#include <cstdio>

ORB_NAMESPACE_BEGIN

void LogInfoString( std::string_view msg )
{

#if defined( ORB_OS_WINDOWS )

	if( IsDebuggerPresent() )
	{
		OutputDebugStringA( msg.data() );
		OutputDebugStringA( "\n" );
	}
	else
	{
		printf( "%s\n", msg.data() );
	}

#elif defined( ORB_OS_ANDROID ) // ORB_OS_WINDOWS

	__android_log_write( ANDROID_LOG_INFO, "Orbit", msg.data() );

#else // ORB_OS_ANDROID

	printf( "%s\n", msg.data() );

#endif // !ORB_OS_ANDROID

}

void LogWarningString( std::string_view msg )
{

#if defined( ORB_OS_WINDOWS )

	if( IsDebuggerPresent() )
	{
		OutputDebugStringA( msg.data() );
		OutputDebugStringA( "\n" );
	}
	else
	{
		HANDLE                     h = GetStdHandle( STD_OUTPUT_HANDLE );
		CONSOLE_SCREEN_BUFFER_INFO old_buffer_info;

		GetConsoleScreenBufferInfo( h, &old_buffer_info );
		SetConsoleTextAttribute( h, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN );
		printf( "%s\n", msg.data() );
		SetConsoleTextAttribute( h, old_buffer_info.wAttributes );
	}

#elif defined( ORB_OS_ANDROID ) // ORB_OS_WINDOWS

	__android_log_write( ANDROID_LOG_WARN, "Orbit", msg.data() );

#else // ORB_OS_ANDROID

	printf( "\x1B[33m%s\x1B[0m\n", msg.data() );

#endif // !ORB_OS_ANDROID

}

void LogErrorString( std::string_view msg )
{

#if defined( ORB_OS_WINDOWS )

	if( IsDebuggerPresent() )
	{
		OutputDebugStringA( msg.data() );
		OutputDebugStringA( "\n" );
	}
	else
	{
		HANDLE                     h = GetStdHandle( STD_OUTPUT_HANDLE );
		CONSOLE_SCREEN_BUFFER_INFO old_buffer_info;

		GetConsoleScreenBufferInfo( h, &old_buffer_info );
		SetConsoleTextAttribute( h, FOREGROUND_INTENSITY | FOREGROUND_RED );
		printf( "%s\n", msg.data() );
		SetConsoleTextAttribute( h, old_buffer_info.wAttributes );
	}

#elif defined( ORB_OS_ANDROID ) // ORB_OS_WINDOWS

	__android_log_write( ANDROID_LOG_ERROR, "Orbit", msg.data() );

#else // ORB_OS_ANDROID

	printf( "\x1B[31m%s\x1B[0m\n", msg.data() );

#endif // !ORB_OS_ANDROID

}

ORB_NAMESPACE_END
