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
#include <variant>

#include "Orbit/Core/Platform/Windows/ComPtr.h"
#include "Orbit/Graphics/Impl/GraphicsAPI.h"
#include "Orbit/Graphics/API/OpenGL/OpenGL.h"

ORB_NAMESPACE_BEGIN

namespace Private
{

#if _ORB_HAS_GRAPHICS_API_OPENGL

	struct _FragmentShaderImplOpenGL
	{
		GLuint id;
	};

#endif
#if _ORB_HAS_GRAPHICS_API_D3D11

	struct _FragmentShaderImplD3D11
	{
		ComPtr< ID3D11PixelShader > pixel_shader;
	};

#endif

	using FragmentShaderImpl = std::variant< std::monostate
	#if _ORB_HAS_GRAPHICS_API_OPENGL
		, _FragmentShaderImplOpenGL
	#endif
	#if _ORB_HAS_GRAPHICS_API_D3D11
		, _FragmentShaderImplD3D11
	#endif
	>;
}

ORB_NAMESPACE_END
