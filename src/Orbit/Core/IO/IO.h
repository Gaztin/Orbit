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
#include "Orbit/Core/Core.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#endif

ORB_NAMESPACE_BEGIN

#if defined( ORB_OS_WINDOWS )

using NativeFileHandle = HANDLE;

inline const NativeFileHandle invalid_file_handle = INVALID_HANDLE_VALUE;

#else

using NativeFileHandle = int;

constexpr int invalid_file_handle = -1;

#endif

ORB_NAMESPACE_END
