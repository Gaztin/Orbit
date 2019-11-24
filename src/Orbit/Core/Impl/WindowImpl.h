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
#include <type_traits>
#include <variant>

#include "Orbit/Core/Impl/WindowAPI.h"

ORB_NAMESPACE_BEGIN

#if _ORB_HAS_WINDOW_API_WIN32
struct _WindowImplWin32
{
	HWND hwnd;
};
#endif
#if _ORB_HAS_WINDOW_API_X11
struct _WindowImplX11
{
	Display* display;
	Window   window;
};
#endif
#if _ORB_HAS_WINDOW_API_WAYLAND
struct _WindowImplWayland
{
	wl_surface* surface;
};
#endif
#if _ORB_HAS_WINDOW_API_COCOA
struct _WindowImplCocoa
{
	void* ns_window;
	void* delegate;
};
#endif
#if _ORB_HAS_WINDOW_API_ANDROID
struct _WindowImplAndroid
{
	ASensorManager*    sensor_manager;
	const ASensor*     accelerometer_sensor;
	ASensorEventQueue* sensor_event_queue;
};
#endif
#if _ORB_HAS_WINDOW_API_UIKIT
struct _WindowImplUIKit
{
	void* ui_window;
};
#endif

using WindowImpl = ::std::variant< ::std::monostate
#if _ORB_HAS_WINDOW_API_WIN32
	, _WindowImplWin32
#endif
#if _ORB_HAS_WINDOW_API_X11
	, _WindowImplX11
#endif
#if _ORB_HAS_WINDOW_API_WAYLAND
	, _WindowImplWayland
#endif
#if _ORB_HAS_WINDOW_API_COCOA
	, _WindowImplCocoa
#endif
#if _ORB_HAS_WINDOW_API_ANDROID
	, _WindowImplAndroid
#endif
#if _ORB_HAS_WINDOW_API_UIKIT
	, _WindowImplUIKit
#endif
>;

ORB_NAMESPACE_END
