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

#include "orbit.h"
#if defined(ORB_OS_WINDOWS)

#include <wtypes.h>
#elif defined(ORB_OS_LINUX)
#include <X11/Xlib.h>
#elif defined(ORB_OS_ANDROID)
#include <android/sensor.h>
#include <android_native_app_glue.h>

#endif

namespace orb
{
namespace platform
{

struct message
{
#if defined(ORB_OS_WINDOWS)
	MSG msg;
#elif defined(ORB_OS_LINUX)
	XEvent xEvent;
#elif defined(ORB_OS_MACOS)
	void* nsEvent; // <NSEvent*>
#elif defined(ORB_OS_ANDROID)
	android_poll_source* source;
	int events;
	ASensorEvent sensorEvent;
#endif
};

}
}
