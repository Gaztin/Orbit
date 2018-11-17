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

/* Per-compiler macros. */
#if defined(_MSC_VER)
#define ORB_CC_MSVC
#define ORB_DLL_EXPORT __declspec(dllexport)
#define ORB_DLL_IMPORT __declspec(dllimport)
#define ORB_DLL_LOCAL
#elif defined(__clang__)
#define ORB_CC_CLANG
#define ORB_DLL_EXPORT __attribute__((visibility("default")))
#define ORB_DLL_IMPORT __attribute__((visibility("default")))
#define ORB_DLL_LOCAL  __attribute__((visibility("hidden")))
#elif defined(__GNUC__)
#define ORB_CC_GCC
#define ORB_DLL_EXPORT __attribute__((visibility("default")))
#define ORB_DLL_IMPORT __attribute__((visibility("default")))
#define ORB_DLL_LOCAL  __attribute__((visibility("hidden")))
#endif

/* Per-system macros. */
#if defined(_WIN32)
#define ORB_OS_WINDOWS
#elif defined(__ANDROID__)
#define ORB_OS_ANDROID
#elif defined(__linux__)
#define ORB_OS_LINUX
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define ORB_OS_IOS
#elif TARGET_OS_MAC
#define ORB_OS_MACOS
#endif
#endif

#if defined(_MSC_VER)
/* Suppress MSVC warnings about DLL-interfaces */
#pragma warning(disable: 4251)
#endif
