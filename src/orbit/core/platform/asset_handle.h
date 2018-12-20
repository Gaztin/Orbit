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

#pragma once
#include <string>

#include "orbit/core.h"

#if defined(ORB_OS_WINDOWS)
#include <windows.h>
#endif

namespace orb
{
namespace platform
{

#if defined(ORB_OS_WINDOWS)
using asset_handle = HANDLE;
#elif defined(ORB_OS_ANDROID)
using asset_handle = AAsset*;
#elif defined(ORB_OS_LINUX) || defined(ORB_OS_MACOS) || defined(ORB_OS_IOS)
using asset_handle = int;
#endif

extern ORB_API_CORE asset_handle open_asset(const std::string& path);
extern ORB_API_CORE size_t get_asset_size(const asset_handle& ah);
extern ORB_API_CORE size_t read_asset_data(const asset_handle& ah, void* buf, size_t size);
extern ORB_API_CORE bool close_asset(const asset_handle& ah);

}
}
