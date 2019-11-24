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
#include "Orbit/Graphics.h"

/* Assume that OpenGL is always available */
#define _ORB_HAS_GRAPHICS_API_OPENGL 1

#if __has_include( <d3d11.h> )
#  define _ORB_HAS_GRAPHICS_API_D3D11 1
#else
#  define _ORB_HAS_GRAPHICS_API_D3D11 0
#endif

#if _ORB_HAS_GRAPHICS_API_D3D11
#  include <d3d11.h>
#endif

ORB_NAMESPACE_BEGIN

enum class GraphicsAPI
{
	Null = 0,
#if _ORB_HAS_GRAPHICS_API_OPENGL
	OpenGL,
#endif
#if _ORB_HAS_GRAPHICS_API_D3D11
	D3D11,
#endif
};

constexpr GraphicsAPI kDefaultGraphicsApi =
#if _ORB_HAS_GRAPHICS_API_D3D11
	GraphicsAPI::D3D11;
#elif _ORB_HAS_GRAPHICS_API_OPENGL
	GraphicsAPI::OpenGL;
#else
	GraphicsAPI::Null;
#endif

ORB_NAMESPACE_END
