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

#include "gl.h"

#include <vector>

#include "orbit/core/utility.h"

#if defined(ORB_OS_LINUX)
#include <GL/glx.h>
#elif defined(ORB_OS_MACOS)
#include <dlfcn.h>
#elif defined(ORB_OS_ANDROID)
#include <EGL/egl.h>
#endif

namespace orb
{
namespace gl
{

namespace platform
{
void* get_proc_address(std::string_view name)
{
#if defined(ORB_OS_WINDOWS)
	return cast<void*>(wglGetProcAddress(name.data()));
#elif defined(ORB_OS_LINUX)
	return cast<void*>(glXGetProcAddress(reinterpret_cast<const GLubyte*>(name.data())));
#elif defined(ORB_OS_MACOS)
	static void* lib = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
	return dlsym(lib, name.data());
#elif defined(ORB_OS_ANDROID)
	return cast<void*>(eglGetProcAddress(name.data()));
#endif
}
}

functions load_functions()
{
	functions fns{};

	/* Buffer objects */
	*cast<void**>(&fns.bind_buffer                ) = platform::get_proc_address("glBindBuffer");
	*cast<void**>(&fns.buffer_data                ) = platform::get_proc_address("glBufferData");
	*cast<void**>(&fns.buffer_sub_data            ) = platform::get_proc_address("glBufferSubData");
	*cast<void**>(&fns.delete_buffers             ) = platform::get_proc_address("glDeleteBuffers");
	*cast<void**>(&fns.disable_vertex_attrib_array) = platform::get_proc_address("glDisableVertexAttribArray");
	*cast<void**>(&fns.draw_arrays                ) = platform::get_proc_address("glDrawArrays");
	*cast<void**>(&fns.draw_elements              ) = platform::get_proc_address("glDrawElements");
	*cast<void**>(&fns.enable_vertex_attrib_array ) = platform::get_proc_address("glEnableVertexAttribArray");
	*cast<void**>(&fns.gen_buffers                ) = platform::get_proc_address("glGenBuffers");
	*cast<void**>(&fns.get_buffer_parameteriv     ) = platform::get_proc_address("glGetBufferParameteriv");
	*cast<void**>(&fns.get_buffer_pointerv        ) = platform::get_proc_address("glBufferPointerv");
	*cast<void**>(&fns.get_vertex_attribdv        ) = platform::get_proc_address("glGetVertexAttribdv");
	*cast<void**>(&fns.get_vertex_attribfv        ) = platform::get_proc_address("glGetVertexAttribfv");
	*cast<void**>(&fns.get_vertex_attribiv        ) = platform::get_proc_address("glGetVertexAttribiv");
	*cast<void**>(&fns.get_vertex_attrib_pointerv ) = platform::get_proc_address("glGetVertexAttribPointerv");
	*cast<void**>(&fns.is_buffer                  ) = platform::get_proc_address("glIsBuffer");
	*cast<void**>(&fns.vertex_attrib1f            ) = platform::get_proc_address("glVertexAttrib1f");
	*cast<void**>(&fns.vertex_attrib2f            ) = platform::get_proc_address("glVertexAttrib2f");
	*cast<void**>(&fns.vertex_attrib3f            ) = platform::get_proc_address("glVertexAttrib3f");
	*cast<void**>(&fns.vertex_attrib4f            ) = platform::get_proc_address("glVertexAttrib4f");
	*cast<void**>(&fns.vertex_attrib_pointer      ) = platform::get_proc_address("glVertexAttribPointer");

	/* Shaders */
	*cast<void**>(&fns.attach_shader       ) = platform::get_proc_address("glAttachShader");
	*cast<void**>(&fns.bind_attrib_location) = platform::get_proc_address("glBindAttribLocation");
	*cast<void**>(&fns.compile_shader      ) = platform::get_proc_address("glCompileShader");
	*cast<void**>(&fns.create_program      ) = platform::get_proc_address("glCreateProgram");
	*cast<void**>(&fns.create_shader       ) = platform::get_proc_address("glCreateShader");
	*cast<void**>(&fns.delete_program      ) = platform::get_proc_address("glDeleteProgram");
	*cast<void**>(&fns.delete_shader       ) = platform::get_proc_address("glDeleteShader");
	*cast<void**>(&fns.detach_shader       ) = platform::get_proc_address("glDetachShader");
	*cast<void**>(&fns.get_active_attrib   ) = platform::get_proc_address("glGetActiveAttrib");
	*cast<void**>(&fns.get_active_uniform  ) = platform::get_proc_address("glGetActiveUniform");
	*cast<void**>(&fns.get_attached_shaders) = platform::get_proc_address("glGetAttachedShaders");
	*cast<void**>(&fns.get_attrib_location ) = platform::get_proc_address("glGetAttribLocation");
	*cast<void**>(&fns.get_program_info_log) = platform::get_proc_address("glGetProgramInfoLog");
	*cast<void**>(&fns.get_programiv       ) = platform::get_proc_address("glGetProgramiv");
	*cast<void**>(&fns.get_shader_info_log ) = platform::get_proc_address("glGetShaderInfoLog");
	*cast<void**>(&fns.get_shader_source   ) = platform::get_proc_address("glGetShaderSource");
	*cast<void**>(&fns.get_shaderiv        ) = platform::get_proc_address("glGetShaderiv");
	*cast<void**>(&fns.get_uniformfv       ) = platform::get_proc_address("glGetUniformfv");
	*cast<void**>(&fns.get_uniformiv       ) = platform::get_proc_address("glGetUniformiv");
	*cast<void**>(&fns.get_uniform_location) = platform::get_proc_address("glGetUniformLocation");
	*cast<void**>(&fns.is_program          ) = platform::get_proc_address("glIsProgram");
	*cast<void**>(&fns.is_shader           ) = platform::get_proc_address("glIsShader");
	*cast<void**>(&fns.link_program        ) = platform::get_proc_address("glLinkProgram");
	*cast<void**>(&fns.shader_source       ) = platform::get_proc_address("glShaderSource");
	*cast<void**>(&fns.uniform1f           ) = platform::get_proc_address("glUniform1f");
	*cast<void**>(&fns.uniform2f           ) = platform::get_proc_address("glUniform2f");
	*cast<void**>(&fns.uniform3f           ) = platform::get_proc_address("glUniform3f");
	*cast<void**>(&fns.uniform4f           ) = platform::get_proc_address("glUniform4f");
	*cast<void**>(&fns.uniform1i           ) = platform::get_proc_address("glUniform1i");
	*cast<void**>(&fns.uniform2i           ) = platform::get_proc_address("glUniform2i");
	*cast<void**>(&fns.uniform3i           ) = platform::get_proc_address("glUniform3i");
	*cast<void**>(&fns.uniform4i           ) = platform::get_proc_address("glUniform4i");
	*cast<void**>(&fns.use_program         ) = platform::get_proc_address("glUseProgram");
	*cast<void**>(&fns.validate_program    ) = platform::get_proc_address("glValidateProgram");

	return fns;
}
}
}
