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

static struct functions_t
{
	~functions_t()
	{
		for (auto pair : pairs)
			*pair.first = nullptr;
	};

	std::vector<std::pair<void**, std::string_view>> pairs;

} s_functions;

template<typename Ret, typename... Args>
static void register_function(Ret(** funPtrAddr)(Args...), std::string_view name)
{
	s_functions.pairs.emplace_back(reinterpret_cast<void**>(funPtrAddr), name);
}

#define REGISTER(X, SYM) decltype(X) X = (register_function(&X, SYM), nullptr)

REGISTER(bind_buffer, "glBindBuffer");
REGISTER(buffer_data, "glBufferData");
REGISTER(buffer_sub_data, "glBufferSubData");
REGISTER(delete_buffers, "glDeleteBuffers");
REGISTER(disable_vertex_attrib_array, "glDisableVertexAttribArray");
REGISTER(draw_arrays, "glDrawArrays");
REGISTER(draw_elements, "glDrawElements");
REGISTER(enable_vertex_attrib_array, "glEnableVertexAttribArray");
REGISTER(gen_buffers, "glGenBuffers");
REGISTER(get_buffer_parameteriv, "glGetBufferParameteriv");
REGISTER(get_buffer_pointerv, "glBufferPointerv");
REGISTER(get_vertex_attribdv, "glGetVertexAttribdv");
REGISTER(get_vertex_attribfv, "glGetVertexAttribfv");
REGISTER(get_vertex_attribiv, "glGetVertexAttribiv");
REGISTER(get_vertex_attrib_pointerv, "glGetVertexAttribPointerv");
REGISTER(is_buffer, "glIsBuffer");
REGISTER(vertex_attrib1f, "glVertexAttrib1f");
REGISTER(vertex_attrib2f, "glVertexAttrib2f");
REGISTER(vertex_attrib3f, "glVertexAttrib3f");
REGISTER(vertex_attrib4f, "glVertexAttrib4f");
REGISTER(vertex_attrib_pointer, "glVertexAttribPointer");

namespace this_platform
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
