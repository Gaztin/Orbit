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

#include "orbit/graphics.h"

/* Assume that OpenGL is always available */
#define __ORB_HAS_GRAPHICS_API_OPENGL 1

#if __has_include( <d3d11.h> )
#  define __ORB_HAS_GRAPHICS_API_D3D11 1
#else
#  define __ORB_HAS_GRAPHICS_API_D3D11 0
#endif

#if __ORB_HAS_GRAPHICS_API_D3D11
#  include <d3d11.h>
#endif

namespace orb
{

#define __ORB_NUM_GRAPHICS_APIS ( __ORB_HAS_GRAPHICS_API_OPENGL + \
                                  __ORB_HAS_GRAPHICS_API_D3D11 )

	enum class graphics_api
	{
		Null = 0,
	#if __ORB_HAS_GRAPHICS_API_OPENGL
		OpenGL,
	#endif
	#if __ORB_HAS_GRAPHICS_API_D3D11
		D3D11,
	#endif
	};

	constexpr graphics_api kDefaultGraphicsApi =
#if __ORB_HAS_GRAPHICS_API_D3D11
		graphics_api::D3D11;
#elif __ORB_HAS_GRAPHICS_API_OPENGL
		graphics_api::OpenGL;
#else
		graphics_api::Null;
#endif

}
