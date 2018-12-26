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
#include "orbit/graphics/platform/opengl/gl.h"
#include "orbit/graphics/platform/render_context_base.h"

namespace orb
{
namespace platform
{
struct window_handle;

class ORB_API_GRAPHICS render_context_gl : public render_context_base
{
public:
	render_context_gl(const window_handle& wh);
	~render_context_gl();

	bool make_current() final override;
	bool make_current(std::nullptr_t) final override;
	void resize(uint32_t width, uint32_t height) final override;
	void swap_buffers() final override;
	void set_clear_color(float r, float g, float b) final override;
	void clear_buffers(buffer_mask mask) final override;

	gl::functions& get_functions() { return m_functions; }

private:
#if defined(ORB_OS_WINDOWS)
		HWND m_parentHwnd;
		HDC m_hdc;
		HGLRC m_hglrc;
#elif defined(ORB_OS_LINUX)
		const struct window_handle* m_wndPtr;
		GC m_gc;
		GLXContext m_glxContext;
#elif defined(ORB_OS_MACOS)
		void* m_glView; // <GLView*>
#elif defined(ORB_OS_ANDROID)
		EGLDisplay m_eglDisplay;
		EGLConfig m_eglConfig;
		EGLSurface m_eglSurface;
		EGLContext m_eglContext;
#elif defined(ORB_OS_IOS)
		void* m_eaglContext; // <EAGLContext*>
		void* m_glkView; // <GLKView*>
#endif

		gl::functions m_functions;
};

}
}
