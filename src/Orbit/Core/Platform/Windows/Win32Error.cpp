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

#include "Win32Error.h"

#if defined( ORB_OS_WINDOWS )
#  include "Orbit/Core/IO/Log.h"

#  include <comdef.h>

ORB_NAMESPACE_BEGIN

bool CheckHResult( HRESULT hresult, std::string_view statement, std::string_view file, uint32_t line )
{
	if( FAILED( hresult ) )
	{
		LPSTR buffer;

		FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hresult, 0, reinterpret_cast< LPSTR >( &buffer ), 0, nullptr );
		LogError( "[%s(%d)] { %s } failed with error: %s", file.data(), line, statement.data(), buffer );

		return false;
	}

	return true;
}

bool CheckSystemError( DWORD error, std::string_view statement, std::string_view file, uint32_t line )
{
	return CheckHResult( HRESULT_FROM_WIN32( error ), statement, file, line );
}

ORB_NAMESPACE_END

#endif // ORB_OS_WINDOWS
