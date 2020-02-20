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

#include "Console.h"

#include "Orbit/Core/Platform/Windows/Win32Error.h"

ORB_NAMESPACE_BEGIN

Console::Console( void )
{

#if defined( ORB_OS_WINDOWS )

	if( ( handle_ = GetStdHandle( STD_OUTPUT_HANDLE ) ) == NULL )
	{
		AllocConsole();
		handle_ = GetStdHandle( STD_OUTPUT_HANDLE );
	}

#endif // ORB_OS_WINDOWS

}

Console::~Console( void )
{

#if defined( ORB_OS_WINDOWS )

	ORB_CHECK_SYSTEM_ERROR( SetStdHandle( STD_OUTPUT_HANDLE, NULL ) );
	FreeConsole();

#endif // ORB_OS_WINDOWS

}

ORB_NAMESPACE_END