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

#include <variant>

#include "orbit/core/memory.h"
#include "orbit/graphics/internal/graphics_api.h"

#if __ORB_HAS_GRAPHICS_API_OPENGL
  #if defined( ORB_OS_WINDOWS )
    #include <gl/GL.h>
  #endif
#endif

namespace orb
{
#if __ORB_HAS_GRAPHICS_API_OPENGL
	struct __vertex_shader_impl_opengl
	{
		GLuint id;
	};
#endif

#if __ORB_HAS_GRAPHICS_API_D3D11
	struct __vertex_shader_impl_d3d11
	{
		com_ptr< ID3DBlob >           vertexData;
		com_ptr< ID3D11VertexShader > vertexShader;
	};
#endif

	using vertex_shader_impl = std::variant< std::monostate
#if __ORB_HAS_GRAPHICS_API_OPENGL
		, __vertex_shader_impl_opengl
#endif
#if __ORB_HAS_GRAPHICS_API_D3D11
		, __vertex_shader_impl_d3d11
#endif
	>;

}
