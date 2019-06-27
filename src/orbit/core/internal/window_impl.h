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

#pragma once

#include <type_traits>
#include <variant>

#include "orbit/core.h"

#if defined( ORB_OS_WINDOWS )
#  define __ORB_HAS_WINDOW_IMPL_WIN32 1
#else
#  define __ORB_HAS_WINDOW_IMPL_WIN32 0
#endif

#if ( __has_include( <X11/Xlib.h> ) )
#  define __ORB_HAS_WINDOW_IMPL_X11 1
#else
#  define __ORB_HAS_WINDOW_IMPL_X11 0
#endif

#if ( __has_include( <wayland-client.h> ) )
#  define __ORB_HAS_WINDOW_IMPL_WAYLAND 1
#else
#  define __ORB_HAS_WINDOW_IMPL_WAYLAND 0
#endif

#if ( __has_include( <Cocoa/Cocoa.h> ) )
#  define __ORB_HAS_WINDOW_IMPL_COCOA 1
#else
#  define __ORB_HAS_WINDOW_IMPL_COCOA 0
#endif

#if defined( ORB_OS_ANDROID )
#  define __ORB_HAS_WINDOW_IMPL_ANDROID 1
#else
#  define __ORB_HAS_WINDOW_IMPL_ANDROID 0
#endif

#if ( __has_include( <UIKit/UIKit.h> ) )
#  define __ORB_HAS_WINDOW_IMPL_UIKIT 1
#else
#  define __ORB_HAS_WINDOW_IMPL_UIKIT 0
#endif

/* Necessary includes */
#if __ORB_HAS_WINDOW_IMPL_WIN32
#  include <Windows.h>
#endif
#if __ORB_HAS_WINDOW_IMPL_X11
#  include <X11/Xlib.h>
#endif
#if __ORB_HAS_WINDOW_IMPL_WAYLAND
#  include <wayland-client.h>
#endif
#if( __ORB_HAS_WINDOW_IMPL_COCOA && defined( __OBJC__ ) )
#  include <Cocoa/Cocoa.h>
#endif
#if __ORB_HAS_WINDOW_IMPL_ANDROID
#  include <android/sensor.h>
#endif

namespace orb
{

#define __ORB_NUM_WINDOW_IMPLS ( __ORB_HAS_WINDOW_IMPL_WIN32   + __ORB_HAS_WINDOW_IMPL_X11   + \
                                 __ORB_HAS_WINDOW_IMPL_WAYLAND + __ORB_HAS_WINDOW_IMPL_COCOA + \
                                 __ORB_HAS_WINDOW_IMPL_ANDROID + __ORB_HAS_WINDOW_IMPL_UIKIT )

	enum class window_impl_type
	{
		Null = 0,
	#if __ORB_HAS_WINDOW_IMPL_WIN32
		Win32,
	#endif
	#if __ORB_HAS_WINDOW_IMPL_X11
		X11,
	#endif
	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
		Wayland,
	#endif
	#if __ORB_HAS_WINDOW_IMPL_COCOA
		Cocoa,
	#endif
	#if __ORB_HAS_WINDOW_IMPL_ANDROID
		Android,
	#endif
	#if __ORB_HAS_WINDOW_IMPL_UIKIT
		UiKit,
	#endif
	};

#if __ORB_HAS_WINDOW_IMPL_WIN32
	struct __window_impl_win32
	{
		HWND hwnd;
	};
#endif
#if __ORB_HAS_WINDOW_IMPL_X11
	struct __window_impl_x11
	{
		Display* display;
		Window   window;
	};
#endif
#if __ORB_HAS_WINDOW_IMPL_WAYLAND
	struct __window_impl_wayland
	{
		wl_surface* surface;
	};
#endif
#if __ORB_HAS_WINDOW_IMPL_COCOA
	struct __window_impl_cocoa
	{
		void* nsWindow;
		void* delegate;
	};
#endif
#if __ORB_HAS_WINDOW_IMPL_ANDROID
	struct __window_impl_android
	{
		ASensorManager*    sensorManager;
		const ASensor*     accelerometerSensor;
		ASensorEventQueue* sensorEventQueue;
	};
#endif
#if __ORB_HAS_WINDOW_IMPL_UIKIT
	struct __window_impl_uikit
	{
		void* uiWindow;
	};
#endif

	using window_impl = ::std::variant< ::std::monostate
	#if __ORB_HAS_WINDOW_IMPL_WIN32
		, __window_impl_win32
	#endif
	#if __ORB_HAS_WINDOW_IMPL_X11
		, __window_impl_x11
	#endif
	#if __ORB_HAS_WINDOW_IMPL_WAYLAND
		, __window_impl_wayland
	#endif
	#if __ORB_HAS_WINDOW_IMPL_COCOA
		, __window_impl_cocoa
	#endif
	#if __ORB_HAS_WINDOW_IMPL_ANDROID
		, __window_impl_android
	#endif
	#if __ORB_HAS_WINDOW_IMPL_UIKIT
		, __window_impl_uikit
	#endif
	>;

	constexpr window_impl_type kDefaultWindowImpl =
	#if __ORB_HAS_WINDOW_IMPL_WIN32
		window_impl_type::Win32;
	#elif __ORB_HAS_WINDOW_IMPL_WAYLAND
		window_impl_type::Wayland;
	#elif __ORB_HAS_WINDOW_IMPL_X11
		window_impl_type::X11;
	#elif __ORB_HAS_WINDOW_IMPL_COCOA
		window_impl_type::Cocoa;
	#elif __ORB_HAS_WINDOW_IMPL_ANDROID
		window_impl_type::Android;
	#elif __ORB_HAS_WINDOW_IMPL_UIKIT
		window_impl_type::UiKit;
	#else
		window_impl_type::Null;
	#endif
}
