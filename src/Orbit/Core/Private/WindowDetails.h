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
#elif defined( ORB_OS_LINUX )
#  include <X11/Xlib.h>
#elif defined( ORB_OS_MACOS )
@class NSWindow;
@class ORB_NAMESPACED_OBJC( WindowDelegate );
#elif defined( ORB_OS_ANDROID )
struct ASensorEventQueue;
struct ASensorManager;
struct ASensor;
#elif defined( ORB_OS_IOS )
@class ORB_NAMESPACE_OBJC( UIWindow );
#endif

ORB_NAMESPACE_BEGIN

namespace Private
{
	struct WindowDetails
	{

#if defined( ORB_OS_WINDOWS )

		HWND hwnd;

#elif defined( ORB_OS_LINUX )

		_XDisplay* display;
		XID        window;

#elif defined( ORB_OS_MACOS )

		NSWindow*                              window;
		ORB_NAMESPACED_OBJC( WindowDelegate )* delegate;

#elif defined( ORB_OS_ANDROID )

		ASensorManager*    sensor_manager;
		const ASensor*     accelerometer_sensor;
		ASensorEventQueue* sensor_event_queue;

#elif defined( ORB_OS_IOS )

		ORB_NAMESPACED_OBJC( UIWindow )* ui_window;

#endif

	};
}

ORB_NAMESPACE_END
