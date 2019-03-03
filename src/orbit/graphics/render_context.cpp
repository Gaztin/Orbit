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

#include "render_context.h"

#include "orbit/core/window.h"
#include "orbit/graphics/platform/d3d11/render_context_d3d11.h"
#include "orbit/graphics/platform/opengl/render_context_gl.h"

namespace orb
{

static render_context* CurrentContext = nullptr;

static std::unique_ptr<platform::render_context_base> init_base(const platform::window_handle& wh, graphics_api api)
{
	switch (api)
	{
#if defined(ORB_HAS_OPENGL)
		case graphics_api::OpenGL_2_0:
			return std::make_unique<platform::render_context_gl>(wh, gl::version::v2_0);

		case graphics_api::OpenGL_3_2:
			return std::make_unique<platform::render_context_gl>(wh, gl::version::v3_2);

		case graphics_api::OpenGL_4_1:
			return std::make_unique<platform::render_context_gl>(wh, gl::version::v4_1);

		case graphics_api::OpenGL_ES_2:
			return std::make_unique<platform::render_context_gl>(wh, gl::version::vES_2);

		case graphics_api::OpenGL_ES_3:
			return std::make_unique<platform::render_context_gl>(wh, gl::version::vES_3);
#endif

#if defined(ORB_HAS_D3D11)
		case graphics_api::Direct3D_11:
			return std::make_unique<platform::render_context_d3d11>(wh);
#endif

		default:
			return nullptr;
	}
}

render_context::render_context(window& parentWindow, graphics_api api)
	: m_api(api)
	, m_parentWindowHandle(parentWindow.get_handle())
	, m_base(init_base(m_parentWindowHandle, m_api))
{
	if (!CurrentContext)
		make_current();

	// Resize context when window is updated
	m_resizeSubscription = parentWindow.subscribe(
		[this](const window_event& e)
	{
		if (e.type == window_event::Resize)
			this->resize(e.data.resize.w, e.data.resize.h);
	});
}

render_context::~render_context()
{
	if (CurrentContext == this)
	{
		m_base->make_current(nullptr);
		CurrentContext = nullptr;
	}
}

bool render_context::make_current()
{
	m_base->make_current();
	CurrentContext = this;
	return true;
}

void render_context::resize(uint32_t width, uint32_t height)
{
	render_context* prev_current_context = CurrentContext;
	make_current();

	m_base->resize(width, height);

	if (prev_current_context != this)
		prev_current_context->make_current();
}

void render_context::swap_buffers()
{
	m_base->swap_buffers();
}

void render_context::clear(buffer_mask mask)
{
	m_base->clear_buffers(mask);
}

void render_context::set_clear_color(float r, float g, float b)
{
	m_base->set_clear_color(r, g, b);
}

render_context* render_context::get_current()
{
	return CurrentContext;
}

}
