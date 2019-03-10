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

#include "shader.h"

#include "orbit/graphics/platform/d3d11/shader_pixel_d3d11.h"
#include "orbit/graphics/platform/d3d11/shader_vertex_d3d11.h"
#include "orbit/graphics/platform/opengl/gl_version.h"
#include "orbit/graphics/platform/opengl/shader_gl.h"
#include "orbit/graphics/render_context.h"

namespace orb
{

static std::unique_ptr<platform::shader_base> init_base(shader_type type, const asset& ast)
{
	orb::graphics_api api = render_context::get_current()->get_api();

	switch (type)
	{
		case shader_type::Vertex:
			switch (api)
			{
#if defined(ORB_HAS_OPENGL)
				case graphics_api::OpenGL_2_0:  return std::make_unique<platform::shader_gl<gl::shader_type::Vertex>>(ast, gl::version::v2_0 );
				case graphics_api::OpenGL_3_2:  return std::make_unique<platform::shader_gl<gl::shader_type::Vertex>>(ast, gl::version::v3_2 );
				case graphics_api::OpenGL_4_1:  return std::make_unique<platform::shader_gl<gl::shader_type::Vertex>>(ast, gl::version::v4_1 );
				case graphics_api::OpenGL_ES_2: return std::make_unique<platform::shader_gl<gl::shader_type::Vertex>>(ast, gl::version::vES_2);
				case graphics_api::OpenGL_ES_3: return std::make_unique<platform::shader_gl<gl::shader_type::Vertex>>(ast, gl::version::vES_3);
#endif
#if defined(ORB_HAS_D3D11)
				case graphics_api::Direct3D_11: return std::make_unique<platform::shader_vertex_d3d11>(ast);
#endif
				default: return nullptr;
			}

		case shader_type::Fragment:
			switch (api)
			{
#if defined(ORB_HAS_OPENGL)
				case graphics_api::OpenGL_2_0:  return std::make_unique<platform::shader_gl<gl::shader_type::Fragment>>(ast, gl::version::v2_0 );
				case graphics_api::OpenGL_3_2:  return std::make_unique<platform::shader_gl<gl::shader_type::Fragment>>(ast, gl::version::v3_2 );
				case graphics_api::OpenGL_4_1:  return std::make_unique<platform::shader_gl<gl::shader_type::Fragment>>(ast, gl::version::v4_1 );
				case graphics_api::OpenGL_ES_2: return std::make_unique<platform::shader_gl<gl::shader_type::Fragment>>(ast, gl::version::vES_2);
				case graphics_api::OpenGL_ES_3: return std::make_unique<platform::shader_gl<gl::shader_type::Fragment>>(ast, gl::version::vES_3);
#endif
#if defined(ORB_HAS_D3D11)
				case graphics_api::Direct3D_11: return std::make_unique<platform::shader_pixel_d3d11>(ast);
#endif
				default: return nullptr;
			}

		default:
			return nullptr;
	}
}

shader::shader(shader_type type, const asset& ast)
	: m_base(init_base(type, ast))
{
}

}
