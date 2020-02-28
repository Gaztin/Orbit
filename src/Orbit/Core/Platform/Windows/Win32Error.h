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
#include "Orbit/Core/Core.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>

#  include <string_view>

ORB_NAMESPACE_BEGIN

extern ORB_API_CORE bool CheckHResult    ( HRESULT hresult, std::string_view statement, std::string_view file, uint32_t line );
extern ORB_API_CORE bool CheckSystemError( DWORD   error,   std::string_view statement, std::string_view file, uint32_t line );

ORB_NAMESPACE_END

#define ORB_CHECK_HRESULT( X )      ( ORB_NAMESPACE CheckHResult( ( X ), #X, __FILE__, __LINE__ ) )
#define ORB_CHECK_SYSTEM_ERROR( X ) ( SetLastError( ERROR_SUCCESS ), ( X ), ORB_NAMESPACE CheckSystemError( GetLastError(), #X, __FILE__, __LINE__ ) )

#endif // ORB_OS_WINDOWS
