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
#elif defined( ORB_OS_LINUX ) || defined( ORB_OS_MACOS ) // ORB_OS_WINDOWS
#  include <unistd.h>
#elif defined( ORB_OS_ANDROID ) // ORB_OS_LINUX || ORB_OS_MACOS
#  include <android/log.h>
#endif // ORB_OS_ANDROID

#include <cstdio>

ORB_NAMESPACE_BEGIN

enum class LogType
{
	Info,
	Warning,
	Error,
};

#if defined( ORB_OS_WINDOWS )

constexpr WORD AttributesByLogType( LogType type )
{
	switch( type )
	{
		case LogType::Info:    return ( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
		case LogType::Warning: return ( FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN );
		case LogType::Error:   return ( FOREGROUND_INTENSITY | FOREGROUND_RED );
		default:               return 0;
	}
}

#elif defined( ORB_OS_LINUX ) || defined( ORB_OS_MACOS ) // ORB_OS_WINDOWS

constexpr int AnsiCodeByLogType( LogType type )
{
	switch( type )
	{
		case LogType::Info:    return 0;
		case LogType::Warning: return 33;
		case LogType::Error:   return 31;
		default:               return 0;
	}
}

#elif defined( ORB_OS_ANDROID ) // ORB_OS_LINUX || ORB_OS_MACOS

constexpr int PriorityByLogType( LogType type )
{
	switch( type )
	{
		case LogType::Info:    return ANDROID_LOG_INFO;
		case LogType::Warning: return ANDROID_LOG_WARN;
		case LogType::Error:   return ANDROID_LOG_ERROR;
		default:               return ANDROID_LOG_UNKNOWN;
	}
}

#endif // ORB_OS_ANDROID

template< LogType Type >
void LogString( std::string_view msg )
{

#if defined( ORB_OS_WINDOWS )

	if( HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE ); ( handle != NULL ) )
	{
		CONSOLE_SCREEN_BUFFER_INFO old_buffer_info;

		GetConsoleScreenBufferInfo( handle, &old_buffer_info );
		SetConsoleTextAttribute( handle, AttributesByLogType( Type ) );
		WriteConsoleA( handle, msg.data(), static_cast< DWORD >( msg.size() ), NULL, NULL );
		SetConsoleTextAttribute( handle, old_buffer_info.wAttributes );
	}
	else if( IsDebuggerPresent() )
	{
		OutputDebugStringA( msg.data() );
		OutputDebugStringA( "\n" );
	}

#elif defined( ORB_OS_LINUX ) || defined( ORB_OS_MACOS ) // ORB_OS_WINDOWS

	const bool is_terminal = ( isatty( STDOUT_FILENO ) != 0 );

	if( is_terminal ) printf( "\x1B[%dm%s\x1B[0m\n", AnsiCodeByLogType( Type ), msg.data() );
	else              printf( "%s\n", msg.data() );

#elif defined( ORB_OS_ANDROID ) // ORB_OS_LINUX || ORB_OS_MACOS

	__android_log_write( AttributesByLogType( Type ), "Orbit", msg.data() );

#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

	printf( "%s\n", msg.data() );

#endif // ORB_OS_IOS

}

void LogInfoString( std::string_view msg )
{
	LogString< LogType::Info >( msg );
}

void LogWarningString( std::string_view msg )
{
	LogString< LogType::Warning >( msg );
}

void LogErrorString( std::string_view msg )
{
	LogString< LogType::Error >( msg );
}

ORB_NAMESPACE_END
