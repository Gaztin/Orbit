/*
* Copyright (c) 2019 Sebastian Kylander http://gaztin.com/
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

#include "constant_buffer.h"

#include "orbit/graphics/platform/d3d11/constant_buffer_d3d11.h"
#include "orbit/graphics/platform/opengl/constant_buffer_gl.h"
#include "orbit/graphics/render_context.h"

namespace orb
{

static std::unique_ptr<platform::constant_buffer_base> init_base(size_t size)
{
	switch (render_context::get_current()->get_api())
	{
		case graphics_api::OpenGL: return std::make_unique<platform::constant_buffer_gl>(size);
		case graphics_api::D3D11: return std::make_unique<platform::constant_buffer_d3d11>(size);
		default: return nullptr;
	}
}

constant_buffer::constant_buffer(size_t size)
	: m_base(init_base(size))
{
}

void constant_buffer::update(const void* data, size_t size)
{
	m_base->update(data, size);
}

void constant_buffer::bind(shader_type type)
{
	m_base->bind(type);
}

}
