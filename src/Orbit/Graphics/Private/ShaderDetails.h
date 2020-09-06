/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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
#include "Orbit/Core/Platform/Windows/ComPtr.h"
#include "Orbit/Graphics/API/OpenGL/OpenGL.h"
#include "Orbit/Graphics/Geometry/VertexLayout.h"

#include <variant>
#include <vector>

ORB_NAMESPACE_BEGIN

namespace Private
{

#if( ORB_HAS_OPENGL )

	struct _ShaderDetailsOpenGL
	{
		struct UniformBlock
		{
			GLuint buffer;
			GLint  referenced_by_vertex_shader;
			GLint  referenced_by_fragment_shader;
		};

		VertexLayout layout;
		GLuint       program;
		GLuint       vao;

		std::vector< UniformBlock > uniform_blocks;
	};

#endif // ORB_HAS_OPENGL
#if( ORB_HAS_D3D11 )

	struct _ShaderDetailsD3D11
	{
		ComPtr< ID3D11VertexShader > vertex_shader;
		ComPtr< ID3D11PixelShader >  pixel_shader;
		ComPtr< ID3D11InputLayout >  input_layout;
		ComPtr< ID3D11SamplerState > sampler_state;

		std::vector< ComPtr< ID3D11Buffer > > vertex_constant_buffers;
		std::vector< ComPtr< ID3D11Buffer > > pixel_constant_buffers;
	};

#endif // ORB_HAS_D3D11

	using ShaderDetails = std::variant< std::monostate
	#if( ORB_HAS_OPENGL )
		, _ShaderDetailsOpenGL
	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )
		, _ShaderDetailsD3D11
	#endif // ORB_HAS_D3D11
	>;
}

ORB_NAMESPACE_END
