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
#include "orbit/core.h"

extern void orbit_main();

namespace orb
{
namespace platform
{

extern ORB_API_CORE void entry_point(int argc, char* argv[]);

}
}

#if defined(ORB_OS_ANDROID)

#include "orbit/core/android_app.h"

void android_main(android_app* app)
{
	orb::android_only::app = app;
	orb::platform::entry_point(0, {});
}

#else

int main(int argc, char* argv[])
{
	orb::platform::entry_point(argc, argv);
	return 0;
}

#endif
