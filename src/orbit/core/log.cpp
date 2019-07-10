/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "log.h"

#if defined( ORB_OS_WINDOWS )
#include <windows.h>
#elif defined( ORB_OS_ANDROID )
#include <android/log.h>
#endif

namespace orb
{
	static void log_info_internal( const char* msg )
	{
	#if defined( ORB_OS_WINDOWS )

		if( IsDebuggerPresent() )
		{
			OutputDebugStringA( msg );
			OutputDebugStringA( "\n" );
		}
		else
		{
			printf( "%s\n", msg );
		}

	#elif defined( ORB_OS_ANDROID )

		__android_log_write( ANDROID_LOG_INFO, "Orbit", msg );

	#else

		printf( "%s\n", msg );

	#endif
	}

	static void log_warning_internal( const char* msg )
	{
	#if defined( ORB_OS_WINDOWS )

		if( IsDebuggerPresent() )
		{
			OutputDebugStringA( msg );
			OutputDebugStringA( "\n" );
		}
		else
		{
			HANDLE                     h = GetStdHandle( STD_OUTPUT_HANDLE );
			CONSOLE_SCREEN_BUFFER_INFO oldBufferInfo;

			GetConsoleScreenBufferInfo( h, &oldBufferInfo );
			SetConsoleTextAttribute( h, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN );
			printf( "%s\n", msg );
			SetConsoleTextAttribute( h, oldBufferInfo.wAttributes );
		}

	#elif defined( ORB_OS_ANDROID )

		__android_log_write( ANDROID_LOG_WARN, "Orbit", msg );

	#else

		printf( "\x1B[33m%s\x1B[0m\n", msg );

	#endif
	}

	static void log_error_internal( const char* msg )
	{
	#if defined( ORB_OS_WINDOWS )

		if( IsDebuggerPresent() )
		{
			OutputDebugStringA( msg );
			OutputDebugStringA( "\n" );
		}
		else
		{
			HANDLE                     h = GetStdHandle( STD_OUTPUT_HANDLE );
			CONSOLE_SCREEN_BUFFER_INFO oldBufferInfo;

			GetConsoleScreenBufferInfo( h, &oldBufferInfo );
			SetConsoleTextAttribute( h, FOREGROUND_INTENSITY | FOREGROUND_RED );
			printf( "%s\n", msg );
			SetConsoleTextAttribute( h, oldBufferInfo.wAttributes );
		}

	#elif defined( ORB_OS_ANDROID )

		__android_log_write( ANDROID_LOG_ERROR, "Orbit", msg );

	#else

		printf( "\x1B[31m%s\x1B[0m\n", msg );

	#endif
	}

	void log_info    ( std::string_view msg ) { log_info_internal( msg.data() ); }
	void log_warning ( std::string_view msg ) { log_warning_internal( msg.data() ); }
	void log_error   ( std::string_view msg ) { log_error_internal( msg.data() ); }
}
