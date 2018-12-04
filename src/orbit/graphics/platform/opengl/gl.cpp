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

functions load_functions()
{
	functions fns{};
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
	return fns;
}

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

}
}
