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
#include <string_view>

#include "orbit/core/asset.h"
#include "orbit/core/log.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/platform/opengl/gl.h"
#include "orbit/graphics/platform/opengl/render_context_gl.h"
#include "orbit/graphics/platform/shader_base.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
namespace platform
{

template<gl::shader_type ShaderType>
class ORB_API_GRAPHICS shader_gl : public shader_base
{
public:
	shader_gl(const asset& ast)
	{
		const auto& fns = static_cast<render_context_gl&>(render_context::get_current()->get_base()).get_functions();
		const auto& data = ast.get_data();
		m_id = fns.create_shader(ShaderType);

#if defined(ORB_OS_LINUX) || defined(ORB_OS_IOS)
		constexpr std::string_view headerString = "#version 300 es\n#define ORB_GLSL 1\n#extension GL_EXT_separate_shader_objects : enable\nprecision highp float;\n";
#elif defined(ORB_OS_ANDROID)
		constexpr std::string_view headerString = "#version 320 es\n#define ORB_GLSL 1\nprecision highp float;\n";
#elif defined(ORB_OS_WINDOWS) || defined(ORB_OS_MACOS)
		constexpr std::string_view headerString = "#version 410\n#define ORB_GLSL 1\n";
#endif
		const GLchar* sources[] = { headerString.data(), cast<const GLchar*>(data.data()) };
		const GLint lengths[] = { static_cast<GLint>(headerString.size()), static_cast<GLint>(data.size()) };
		fns.shader_source(m_id, count_of(sources), sources, lengths);
		fns.compile_shader(m_id);

		GLint loglen = 0;
		fns.get_shaderiv(m_id, gl::shader_param::InfoLogLength, &loglen);
		if (loglen > 0)
		{
			std::string logbuf(static_cast<size_t>(loglen), '\0');
			fns.get_shader_info_log(m_id, loglen, nullptr, &logbuf[0]);
			log_error(logbuf);
		}
	}

	shader_type get_type() const final override;

	GLuint get_id() const { return m_id; }

private:
	GLuint m_id;
};

template<>
inline shader_type shader_gl<gl::shader_type::Vertex>::get_type() const
{
	return shader_type::Vertex;
}

template<>
inline shader_type shader_gl<gl::shader_type::Fragment>::get_type() const
{
	return shader_type::Fragment;
}

}
}
