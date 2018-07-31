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

#include "window_impl.h"

#include <cassert>

#include "orbit/core/android_app.h"
#include "orbit/core/log.h"
#include "orbit/core/utility.h"

namespace orb
{

static int numWindows = 0;

window_impl::window_impl()
	: m_open(false)
{
	/* Android only allows for the single window. */
	assert(++numWindows == 1);

	android_only::app->onAppCmd     = &window_impl::app_cmd;
	android_only::app->onInputEvent = &window_impl::input_event;
	android_only::app->userData     = this;

	/* Update until native window is initialized. */
	while (!m_open)
		poll_events();
}

window_impl::window_impl(uint32_t width, uint32_t height) : window_impl() { }

window_impl::~window_impl()
{
	--numWindows;
}

void window_impl::poll_events()
{
	android_poll_source* source;
	int                  events;
	while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0)
	{
		if (source)
			source->process(android_only::app, source);
	}
}

void window_impl::set_title(const std::string& title)
{
}

void window_impl::set_pos(uint32_t x, uint32_t y)
{
}

void window_impl::set_size(uint32_t width, uint32_t height)
{
}

void window_impl::set_visible(bool visible)
{
}

void window_impl::app_cmd(android_app* state, int cmd)
{
	window_impl& w = *cast<window_impl*>(state->userData);
	switch (cmd)
	{
		case APP_CMD_INIT_WINDOW:
			w.m_open = true;
			break;

		case APP_CMD_DESTROY:
			w.m_open = false;
			break;

		default:
			break;
	}
}

int window_impl::input_event(android_app* state, AInputEvent *e)
{
	switch (AInputEvent_getType(e))
	{
		default:
			break;
	}

	return 0;
}

}
