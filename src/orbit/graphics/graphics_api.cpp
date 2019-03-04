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

#include "graphics_api.h"

namespace orb
{

graphics_api get_system_default_graphics_api()
{
#if defined(ORB_OS_WINDOWS)
	return graphics_api::Direct3D_11;
#elif defined(ORB_OS_LINUX)
	return graphics_api::OpenGL_3_2;
#elif defined(ORB_OS_MACOS)
	return graphics_api::OpenGL_2_0;
#elif defined(ORB_OS_ANDROID)
	return graphics_api::OpenGL_ES_3;
#elif defined(ORB_OS_IOS)
	return graphics_api::OpenGL_ES_2;
#endif
}

}
