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

#include <map>
#include <vector>

#include "orbit/core/log.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/platform/opengl/render_context_gl.h"
#include "orbit/graphics/render_context.h"

#if defined(ORB_OS_LINUX)
#include <GL/glx.h>
#elif defined(ORB_OS_MACOS)
#include <dlfcn.h>
#elif defined(ORB_OS_ANDROID)
#include <EGL/egl.h>
#elif defined(ORB_OS_IOS)
#include <dlfcn.h>
#endif

namespace orb
{
namespace gl
{

const std::map<GLenum, std::string_view> kErrorCodes =
{
	{ GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument. The offending command is ignored"
	                   " and has no other side effect than to set the error flag." },
	{ GL_INVALID_VALUE, "A numeric argument is out of range. The offending command is ignored and has no other side"
	                    " effect than to set the error flag." },
	{ GL_INVALID_OPERATION, "The specified operation is not allowed in the current state. The offending command is"
	                        " ignored and has no other side effect than to set the error flag." },
	{ GL_STACK_OVERFLOW, "This command would cause a stack overflow. The offending command is ignored and has no other"
	                     " side effect than to set the error flag." },
	{ GL_STACK_UNDERFLOW, "This command would cause a stack underflow. The offending command is ignored and has no other"
	                      " side effect than to set the error flag." },
	{ GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command. The state of the GL is undefined,"
	                    " except for the state of the error flags, after this error is recorded." },
#if defined(GL_TABLE_TOO_LARGE)
	{ GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size. The offending"
	                      " command is ignored and has no other side effect than to set the error flag." },
#endif
#if defined(GL_INVALID_FRAMEBUFFER_OPERATION)
	{ GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete. The offending command is ignored and"
	                                    " has no other side effect than to set the error flag." },
#endif
};

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
#elif defined(ORB_OS_IOS)
	return dlsym(RTLD_DEFAULT, name.data());
#endif
}
}

void handle_error(GLenum err)
{
	if (err == GL_NO_ERROR)
		return;
		
	auto it = kErrorCodes.find(err);
	if (it != kErrorCodes.end())
	{
		orb::log_error(it->second);
	}
	else
	{
		orb::log_error(format_view("Unknown error: %d", err));
	}
}

functions& get_current_functions()
{
	thread_local gl::functions defaultFunctions;

	render_context* ctx = render_context::get_current();
	if (!ctx)
		return defaultFunctions;

	switch (ctx->get_api())
	{
		case graphics_api::OpenGL_2_0:
		case graphics_api::OpenGL_3_2:
		case graphics_api::OpenGL_4_1:
		case graphics_api::OpenGL_ES_2:
		case graphics_api::OpenGL_ES_3:
			return static_cast<orb::platform::render_context_gl*>(&ctx->get_base())->get_functions();

		case graphics_api::Direct3D_11:
			return defaultFunctions;
	}

	return defaultFunctions;
}

}
}
