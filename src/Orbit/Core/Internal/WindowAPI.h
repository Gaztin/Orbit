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
#include "Orbit/Core.h"

#if defined( ORB_OS_WINDOWS )
#  define _ORB_HAS_WINDOW_API_WIN32 1
#else
#  define _ORB_HAS_WINDOW_API_WIN32 0
#endif

#if ( __has_include( <X11/Xlib.h> ) )
#  define _ORB_HAS_WINDOW_API_X11 1
#else
#  define _ORB_HAS_WINDOW_API_X11 0
#endif

#if ( __has_include( <wayland-client.h> ) )
#  define _ORB_HAS_WINDOW_API_WAYLAND 1
#else
#  define _ORB_HAS_WINDOW_API_WAYLAND 0
#endif

#if ( __has_include( <Cocoa/Cocoa.h> ) )
#  define _ORB_HAS_WINDOW_API_COCOA 1
#else
#  define _ORB_HAS_WINDOW_API_COCOA 0
#endif

#if defined( ORB_OS_ANDROID )
#  define _ORB_HAS_WINDOW_API_ANDROID 1
#else
#  define _ORB_HAS_WINDOW_API_ANDROID 0
#endif

#if ( __has_include( <UIKit/UIKit.h> ) )
#  define _ORB_HAS_WINDOW_API_UIKIT 1
#else
#  define _ORB_HAS_WINDOW_API_UIKIT 0
#endif

/* Necessary includes */
#if _ORB_HAS_WINDOW_API_WIN32
#  include <Windows.h>
#endif
#if _ORB_HAS_WINDOW_API_X11
#  include <X11/Xlib.h>
#endif
#if _ORB_HAS_WINDOW_API_WAYLAND
#  include <wayland-client.h>
#endif
#if( _ORB_HAS_WINDOW_API_COCOA && defined( __OBJC__ ) )
#  include <Cocoa/Cocoa.h>
#endif
#if _ORB_HAS_WINDOW_API_ANDROID
#  include <android/sensor.h>
#endif

ORB_NAMESPACE_BEGIN

enum class WindowAPI
{
	Null = 0,
#if _ORB_HAS_WINDOW_API_WIN32
	Win32,
#endif
#if _ORB_HAS_WINDOW_API_X11
	X11,
#endif
#if _ORB_HAS_WINDOW_API_WAYLAND
	Wayland,
#endif
#if _ORB_HAS_WINDOW_API_COCOA
	Cocoa,
#endif
#if _ORB_HAS_WINDOW_API_ANDROID
	Android,
#endif
#if _ORB_HAS_WINDOW_API_UIKIT
	UiKit,
#endif
};

constexpr WindowAPI kDefaultWindowApi =
#if _ORB_HAS_WINDOW_API_WIN32
	WindowAPI::Win32;
#elif _ORB_HAS_WINDOW_API_WAYLAND
	WindowAPI::Wayland;
#elif _ORB_HAS_WINDOW_API_X11
	WindowAPI::X11;
#elif _ORB_HAS_WINDOW_API_COCOA
	WindowAPI::Cocoa;
#elif _ORB_HAS_WINDOW_API_ANDROID
	WindowAPI::Android;
#elif _ORB_HAS_WINDOW_API_UIKIT
	WindowAPI::UiKit;
#else
	WindowAPI::Null;
#endif

ORB_NAMESPACE_END
