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

#include "graphics_pipeline_gl.h"

#include "orbit/graphics/platform/opengl/buffer_gl.h"
#include "orbit/graphics/platform/opengl/shader_gl.h"
#include "orbit/graphics/index_buffer.h"
#include "orbit/graphics/shader.h"
#include "orbit/graphics/vertex_buffer.h"

namespace orb
{
namespace platform
{

static gl::vertex_attrib_data_type get_vertex_component_data_type(vertex_component::type_t type)
{
	switch (type)
	{
		case vertex_component::Float:
		case vertex_component::Vec2:
		case vertex_component::Vec3:
		case vertex_component::Vec4:
			return gl::vertex_attrib_data_type::Float;

		default:
			return gl::vertex_attrib_data_type::Float;
	}
}

static GLint get_vertex_component_length(vertex_component::type_t type)
{
	switch (type)
	{
		case vertex_component::Float: return 1;
		case vertex_component::Vec2: return 2;
		case vertex_component::Vec3: return 3;
		case vertex_component::Vec4: return 4;
		default: return 0;
	}
}

static GLsizei get_data_type_size(gl::vertex_attrib_data_type type)
{
	switch (type)
	{
		case gl::vertex_attrib_data_type::Byte: return 1;
		case gl::vertex_attrib_data_type::UnsignedByte: return 1;
		case gl::vertex_attrib_data_type::Short: return sizeof(short);
		case gl::vertex_attrib_data_type::UnsignedShort: return sizeof(unsigned short);
		case gl::vertex_attrib_data_type::Float: return sizeof(float);
		default: return 0;
	}
}

static gl::index_type get_index_type(index_format fmt)
{
	switch (fmt)
	{
		case index_format::Byte: return gl::index_type::Byte;
		case index_format::Word: return gl::index_type::Short;
		case index_format::DoubleWord: return gl::index_type::Int;
		default: return static_cast<gl::index_type>(0);
	}
}

graphics_pipeline_gl::graphics_pipeline_gl()
{
	const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
	m_programId = fns.create_program();
}

graphics_pipeline_gl::~graphics_pipeline_gl()
{
	const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
	fns.delete_program(m_programId);
}

void graphics_pipeline_gl::bind()
{
	const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
	fns.use_program(m_programId);

	const uint8_t* pointer = nullptr;
	for (GLuint i = 0; i < m_layout.size(); ++i)
	{
		const gl::vertex_attrib_data_type data_type = get_vertex_component_data_type(m_layout[i].type);
		const GLint length = get_vertex_component_length(m_layout[i].type);
		fns.enable_vertex_attrib_array(i);
		fns.vertex_attrib_pointer(i, length, data_type, GL_FALSE, m_stride, pointer);
		pointer += length * get_data_type_size(data_type);
	}
}

void graphics_pipeline_gl::unbind()
{
	const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
	for (GLuint i = 0; i < m_layout.size(); ++i)
		fns.disable_vertex_attrib_array(i);

	fns.use_program(0);
}

void graphics_pipeline_gl::add_shader(const shader& shr)
{
	GLuint shader_id = 0;
	switch (shr.get_type())
	{
		case shader_type::Vertex:
			shader_id = reinterpret_cast<const shader_gl<gl::shader_type::Vertex>&>(shr.get_base()).get_id();
			break;

		case shader_type::Fragment:
			shader_id = reinterpret_cast<const shader_gl<gl::shader_type::Fragment>&>(shr.get_base()).get_id();
			break;

		default:
			return;
	}

	const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
	fns.attach_shader(m_programId, shader_id);
	fns.link_program(m_programId);

	GLint loglen = 0;
	fns.get_programiv(m_programId, gl::program_param::InfoLogLength, &loglen);
	if (loglen > 0)
	{
		std::string logbuf(static_cast<size_t>(loglen), '\0');
		fns.get_program_info_log(m_programId, loglen, nullptr, &logbuf[0]);
		log_error(logbuf);
	}
}

void graphics_pipeline_gl::describe_vertex_layout(vertex_layout layout)
{
	m_layout.assign(layout);

	/* Calculate stride */
	m_stride = 0;
	for (const vertex_component& cmp : m_layout)
		m_stride += get_vertex_component_length(cmp.type) * get_data_type_size(get_vertex_component_data_type(cmp.type));
}

void graphics_pipeline_gl::draw(const vertex_buffer& vb)
{
	const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
	fns.draw_arrays(gl::draw_mode::Triangles, 0, static_cast<GLsizei>(vb.get_count()));
}

void graphics_pipeline_gl::draw(const index_buffer& ib)
{
	const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
	fns.draw_elements(gl::draw_mode::Triangles, static_cast<GLsizei>(ib.get_count()), get_index_type(ib.get_format()), nullptr);
}

}
}
