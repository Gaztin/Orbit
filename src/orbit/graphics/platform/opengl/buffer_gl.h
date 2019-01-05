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
#include <cstddef>

#include "orbit/graphics/platform/opengl/gl.h"
#include "orbit/graphics/platform/opengl/render_context_gl.h"
#include "orbit/graphics/platform/buffer_base.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
namespace platform
{

template<gl::buffer_target BufferTarget>
class buffer_gl : public buffer_base
{
public:
	buffer_gl(const void* data, size_t size)
		: m_id(0)
	{
		const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
		fns.gen_buffers(1, &m_id);
		fns.bind_buffer(BufferTarget, m_id);
		fns.buffer_data(BufferTarget, size, data, orb::gl::buffer_usage::StaticDraw);
		fns.bind_buffer(BufferTarget, 0);
	}

	~buffer_gl()
	{
		const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
		fns.delete_buffers(1, &m_id);
	}

	void bind() final override
	{
		const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
		fns.bind_buffer(BufferTarget, m_id);
	}

private:
	GLuint m_id;
};

}
}
