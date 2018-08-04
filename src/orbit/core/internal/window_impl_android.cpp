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
#include <orbit/core/events/window_event.h>

#include "orbit/core/android_app.h"
#include "orbit/core/events/window_event.h"
#include "orbit/core/log.h"
#include "orbit/core/utility.h"

namespace orb
{

static int numWindows = 0;

static event_dispatcher<window_event> defaultEventDispatcher;

window_impl::window_impl()
	: m_open(false)
	, m_eventDispatcher(&defaultEventDispatcher)
{
	/* Android only allows for the single window. */
	assert(++numWindows == 1);

	android_only::app->onAppCmd = &window_impl::app_cmd;
	android_only::app->onInputEvent = &window_impl::input_event;
	android_only::app->activity->callbacks->onContentRectChanged = &window_impl::on_content_rect_changed;
	android_only::app->userData = this;

	m_sensorManager = ASensorManager_getInstance();
	m_accelerometerSensor = ASensorManager_getDefaultSensor(m_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	m_sensorEventQueue = ASensorManager_createEventQueue(m_sensorManager, android_only::app->looper, LOOPER_ID_USER, nullptr, nullptr);

	/* Update until native window is initialized. */
	while (!m_open)
		poll_events();
}

window_impl::window_impl(uint32_t width, uint32_t height) : window_impl() { }

window_impl::~window_impl()
{
	ASensorEventQueue_disableSensor(m_sensorEventQueue, m_accelerometerSensor);
	ASensorManager_destroyEventQueue(m_sensorManager, m_sensorEventQueue);

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

		ASensorEvent sensorEvent;
		while (ASensorEventQueue_getEvents(m_sensorEventQueue, &sensorEvent, 1) > 0)
		{
		}
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
			w.m_eventDispatcher->queue_event({window_event::Restore});
			break;

		case APP_CMD_TERM_WINDOW:
			w.m_eventDispatcher->queue_event({window_event::Suspend});
			break;

		case APP_CMD_GAINED_FOCUS:
			ASensorEventQueue_enableSensor(w.m_sensorEventQueue, w.m_accelerometerSensor);
			ASensorEventQueue_setEventRate(w.m_sensorEventQueue, w.m_accelerometerSensor, (1000 * 1000 / 60));
			w.m_eventDispatcher->queue_event({window_event::Focus});
			break;

		case APP_CMD_LOST_FOCUS:
			ASensorEventQueue_disableSensor(w.m_sensorEventQueue, w.m_accelerometerSensor);
			w.m_eventDispatcher->queue_event({window_event::Defocus});
			break;

		case APP_CMD_DESTROY:
			w.m_open = false;
			break;

		default:
			break;
	}
}

int window_impl::input_event(android_app* state, AInputEvent* e)
{
	switch (AInputEvent_getType(e))
	{
		default:
			break;
	}

	return 0;
}

void window_impl::on_content_rect_changed(ANativeActivity* activity, const ARect* rect)
{
	window_event e;
	e.type = window_event::Resize;
	e.data.resize.w = cast<uint32_t>(rect->right - rect->left);
	e.data.resize.h = cast<uint32_t>(rect->bottom - rect->top);
	cast<window_impl*>(android_only::app->userData)->m_eventDispatcher->queue_event(e);
}

}
