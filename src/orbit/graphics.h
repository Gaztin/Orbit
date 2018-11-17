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

#include "core/bitmask.h"

#if defined(ORB_BUILD_GRAPHICS)
#define ORB_API_GRAPHICS ORB_DLL_EXPORT
#else
#define ORB_API_GRAPHICS ORB_DLL_IMPORT
#endif

/* Graphics API macros. */
#if defined(ORB_OS_WINDOWS)
#define ORB_HAS_D3D11
#define ORB_HAS_OPENGL
#elif defined(ORB_OS_LINUX)
#define ORB_HAS_OPENGL
#elif defined(ORB_OS_MACOS)
#define ORB_HAS_OPENGL
#elif defined(ORB_OS_ANDROID)
#define ORB_HAS_OPENGL
#elif defined(ORB_OS_IOS)
#define ORB_HAS_OPENGL
#endif

/* Enumerators */

namespace orb
{

enum class graphics_api
{
	OpenGL,
	D3D11,

#if defined(ORB_OS_WINDOWS)
	DeviceDefault = D3D11,
#else
	DeviceDefault = OpenGL,
#endif
};

enum class buffer_mask
{
	Color = 0x1,
	Depth = 0x2,
};
ORB_ENABLE_BITMASKING(buffer_mask);

}
