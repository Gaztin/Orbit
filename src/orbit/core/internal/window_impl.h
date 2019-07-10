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

#include "orbit/core/internal/window_api.h"

namespace orb
{
#if __ORB_HAS_WINDOW_API_WIN32
	struct __window_impl_win32
	{
		HWND hwnd;
	};
#endif
#if __ORB_HAS_WINDOW_API_X11
	struct __window_impl_x11
	{
		Display* display;
		Window   window;
	};
#endif
#if __ORB_HAS_WINDOW_API_WAYLAND
	struct __window_impl_wayland
	{
		wl_surface* surface;
	};
#endif
#if __ORB_HAS_WINDOW_API_COCOA
	struct __window_impl_cocoa
	{
		void* nsWindow;
		void* delegate;
	};
#endif
#if __ORB_HAS_WINDOW_API_ANDROID
	struct __window_impl_android
	{
		ASensorManager*    sensorManager;
		const ASensor*     accelerometerSensor;
		ASensorEventQueue* sensorEventQueue;
	};
#endif
#if __ORB_HAS_WINDOW_API_UIKIT
	struct __window_impl_uikit
	{
		void* uiWindow;
	};
#endif

	using window_impl = ::std::variant< ::std::monostate
#if __ORB_HAS_WINDOW_API_WIN32
		, __window_impl_win32
#endif
#if __ORB_HAS_WINDOW_API_X11
		, __window_impl_x11
#endif
#if __ORB_HAS_WINDOW_API_WAYLAND
		, __window_impl_wayland
#endif
#if __ORB_HAS_WINDOW_API_COCOA
		, __window_impl_cocoa
#endif
#if __ORB_HAS_WINDOW_API_ANDROID
		, __window_impl_android
#endif
#if __ORB_HAS_WINDOW_API_UIKIT
		, __window_impl_uikit
#endif
	>;

}
