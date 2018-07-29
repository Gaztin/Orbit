/*
* Copyright (c) 2018 Sebastian Kylander http://gaztin.com/
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

#if defined(ORB_OS_WINDOWS)
#include <windows.h>
#elif defined(ORB_OS_ANDROID)
#include <android/log.h>
#endif

namespace orb
{

void log_info(const std::string& msg)
{
#if defined(ORB_OS_ANDROID)
	__android_log_write(ANDROID_LOG_INFO, "Orbit", msg.c_str());
#else
	printf("%s\n", msg.c_str());
#endif
}

void log_warning(const std::string& msg)
{
#if defined(ORB_OS_WINDOWS)
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO oldBufferInfo;
	GetConsoleScreenBufferInfo(h, &oldBufferInfo);
	SetConsoleTextAttribute(h, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
	printf("%s\n", msg.c_str());
	SetConsoleTextAttribute(h, oldBufferInfo.wAttributes);

#elif defined(ORB_OS_ANDROID)
	__android_log_write(ANDROID_LOG_WARNING, "Orbit", msg.c_str());

#else
	printf("\x1B[33m%s\x1B[0m\n", msg.c_str());
#endif
}

void log_error(const std::string& msg)
{
#if defined(ORB_OS_WINDOWS)
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO oldBufferInfo;
	GetConsoleScreenBufferInfo(h, &oldBufferInfo);
	SetConsoleTextAttribute(h, FOREGROUND_INTENSITY | FOREGROUND_RED);
	printf("%s\n", msg.c_str());
	SetConsoleTextAttribute(h, oldBufferInfo.wAttributes);

#elif defined(ORB_OS_ANDROID)
	__android_log_write(ANDROID_LOG_ERROR, "Orbit", msg.c_str());

#else
	printf("\x1B[31m%s\x1B[0m\n", msg.c_str());
#endif
}

}
