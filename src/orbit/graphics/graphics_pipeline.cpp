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

#include "graphics_pipeline.h"

#include "orbit/graphics/render_context.h"
#include "orbit/graphics/platform/d3d11/graphics_pipeline_d3d11.h"
#include "orbit/graphics/platform/opengl/graphics_pipeline_gl.h"

namespace orb
{

static std::unique_ptr<platform::graphics_pipeline_base> init_base()
{
	switch (render_context::get_current()->get_api())
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL_2_0:
		case graphics_api::OpenGL_3_2:
		case graphics_api::OpenGL_4_1:
		case graphics_api::OpenGL_ES_2:
		case graphics_api::OpenGL_ES_3:
			return std::make_unique<platform::graphics_pipeline_gl>();
#endif

#if defined(ORB_HAS_D3D11)
		case graphics_api::Direct3D_11:
			return std::make_unique<platform::graphics_pipeline_d3d11>();
#endif

		default:
			return nullptr;
	}
}

graphics_pipeline::graphics_pipeline()
	: m_base(init_base())
{
}

void graphics_pipeline::add_shader(const shader& shr)
{
	m_base->add_shader(shr);
}

void graphics_pipeline::describe_vertex_layout(vertex_layout layout)
{
	m_base->describe_vertex_layout(layout);
}

void graphics_pipeline::draw(size_t vertexCount)
{
	m_base->draw(vertexCount);
}

}
