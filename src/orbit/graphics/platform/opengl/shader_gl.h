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
private:
	struct traits
	{
		static std::string_view make_varying_macro(gl::version v);
		static std::string_view make_attribute_macro(gl::version v);
		static std::string_view make_out_color_declaration(gl::version v);
	};

public:
	shader_gl(const asset& ast, gl::version v)
	{
		auto& gl = gl::get_current_functions();
		m_id = gl.create_shader(ShaderType);

		std::string_view headerStr;
		switch (v)
		{
			case gl::version::v2_0:  headerStr = "#version 110\n#define ORB_GLSL 1\n"; break;
			case gl::version::v3_2:  headerStr = "#version 150\n#define ORB_GLSL 1\n"; break;
			case gl::version::v4_1:  headerStr = "#version 410\n#define ORB_GLSL 1\n"; break;
			case gl::version::vES_2: headerStr = "#version 100\n#define ORB_GLSL 1\nprecision highp float;\n"; break;
			case gl::version::vES_3: headerStr = "#version 300 es\n#define ORB_GLSL 1\nprecision highp float;\n"; break;
		}

		std::string_view constantsMacrosStr;
		switch (v)
		{
			case orb::gl::version::v2_0:
			case orb::gl::version::vES_2:
				constantsMacrosStr = "#define ORB_CONSTANTS_BEGIN(X)\n#define ORB_CONSTANTS_END\n#define ORB_CONSTANT(T, N) uniform T N;\n";
				break;

			case orb::gl::version::v3_2:
			case orb::gl::version::v4_1:
			case orb::gl::version::vES_3:
				constantsMacrosStr = "#define ORB_CONSTANTS_BEGIN(X) layout (std140) uniform X {\n#define ORB_CONSTANTS_END };\n#define ORB_CONSTANT(T, N) T N;\n"; break;
				break;
		}

		const std::string_view varyingStr      = traits::make_varying_macro(v);
		const std::string_view attributeStr    = traits::make_attribute_macro(v);
		const std::string_view outColorDeclStr = traits::make_out_color_declaration(v);
		const auto&            data            = ast.get_data();

		const GLchar* sources[] =
		{
			headerStr.data(),
			constantsMacrosStr.data(),
			varyingStr.data(),
			attributeStr.data(),
			outColorDeclStr.data(),
			cast<const GLchar*>(data.data()),
		};
		const GLint lengths[] =
		{
			static_cast<GLint>(headerStr.size()),
			static_cast<GLint>(constantsMacrosStr.size()),
			static_cast<GLint>(varyingStr.size()),
			static_cast<GLint>(attributeStr.size()),
			static_cast<GLint>(outColorDeclStr.size()),
			static_cast<GLint>(data.size()),
		};
		gl.shader_source(m_id, count_of(sources), sources, lengths);
		gl.compile_shader(m_id);

		GLint loglen = 0;
		gl.get_shaderiv(m_id, gl::shader_param::InfoLogLength, &loglen);
		if (loglen > 0)
		{
			std::string logbuf(static_cast<size_t>(loglen), '\0');
			gl.get_shader_info_log(m_id, loglen, nullptr, &logbuf[0]);
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

template<>
struct shader_gl<gl::shader_type::Vertex>::traits
{
	static std::string_view make_varying_macro(gl::version v)
	{
		switch (v)
		{
			case gl::version::v2_0:
			case gl::version::vES_2:
				return "#define ORB_VARYING varying\n";

			case gl::version::v3_2:
			case gl::version::v4_1:
			case gl::version::vES_3:
				return "#define ORB_VARYING out\n";

			default:
				return "";
		}
	}

	static std::string_view make_attribute_macro(gl::version v)
	{
		switch (v)
		{
			case gl::version::v2_0:
			case gl::version::vES_2:
				return "#define ORB_ATTRIBUTE attribute\n";

			case gl::version::v3_2:
			case gl::version::v4_1:
			case gl::version::vES_3:
				return "#define ORB_ATTRIBUTE in\n";

			default:
				return "";
		}
	}

	static std::string_view make_out_color_declaration(gl::version)
	{
		return "";
	}
};

template<>
struct shader_gl<gl::shader_type::Fragment>::traits
{
	static std::string_view make_varying_macro(gl::version v)
	{
		switch (v)
		{
			case gl::version::v2_0:
			case gl::version::vES_2:
				return "#define ORB_VARYING varying\n";

			case gl::version::v3_2:
			case gl::version::v4_1:
			case gl::version::vES_3:
				return "#define ORB_VARYING in\n";

			default:
				return "";
		}
	}

	static std::string_view make_attribute_macro(gl::version v)
	{
		switch (v)
		{
			case gl::version::v2_0:
			case gl::version::vES_2:
				return "#define ORB_ATTRIBUTE attribute\n";

			case gl::version::v3_2:
			case gl::version::v4_1:
			case gl::version::vES_3:
			default:
				return "";
		}
	}

	static std::string_view make_out_color_declaration(gl::version v)
	{
		switch (v)
		{
			case gl::version::v2_0:
			case gl::version::vES_2:
			default:
				return "#define ORB_SET_OUT_COLOR(X) gl_FragColor = X\n";

			case gl::version::v3_2:
			case gl::version::v4_1:
			case gl::version::vES_3:
				return "out vec4 orb_outColor;\n#define ORB_SET_OUT_COLOR(X) orb_outColor = X\n";
		}
	}
};

}
}
