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
#include "Orbit/Core/Application/Application.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>

INT WINAPI WinMain( HINSTANCE, HINSTANCE, PSTR, INT )
{
	ORB_NAMESPACE ApplicationBase::RunInstance();
	return 0;
}

#elif defined( ORB_OS_LINUX ) || defined( ORB_OS_MACOS ) || defined( ORB_OS_IOS )

int main( int, char*[] )
{
	ORB_NAMESPACE ApplicationBase::RunInstance();
	return 0;
}

#elif defined( ORB_OS_ANDROID )
#  include "orbit/Core/AndroidApp.h"

extern "C" void android_main( android_app* app )
{
	ORB_NAMESPACE AndroidOnly::app = app;
	ORB_NAMESPACE ApplicationBase::RunInstance();
}

#endif
