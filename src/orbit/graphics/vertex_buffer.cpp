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

#include "vertex_buffer.h"

#include <assert.h>

#include "orbit/graphics/render_context.h"
#include "platform/d3d11/buffer_d3d11.h"
#include "platform/opengl/buffer_gl.h"

namespace orb
{

static std::unique_ptr<platform::buffer_base> init_base(const void* data, size_t count, size_t size)
{
	switch (render_context::get_current()->get_api())
	{
		case graphics_api::OpenGL: return std::make_unique<platform::buffer_gl<gl::buffer_target::Array>>(data, count, size);
		case graphics_api::D3D11: return std::make_unique<platform::buffer_d3d11<d3d11::bind_flag::VertexBuffer>>(data, count, size);
		default: return nullptr;
	}
}

vertex_buffer::vertex_buffer(const void* data, size_t count, size_t size)
	: m_base(init_base(data, count, size))
{
}

void vertex_buffer::bind()
{
	m_base->bind();
}

}
