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

#include "core_platform.h"

#include <android_native_app_glue.h>
#include <orbit/core/events/window_event.h>

#include "orbit/core/platform/message.h"
#include "orbit/core/android_app.h"
#include "orbit/core/utility.h"
#include "orbit/core/window.h"

namespace orb
{
namespace platform
{

static void app_cmd(android_app* state, int cmd)
{
	window& wnd = *cast<window*>(state->userData);
	switch (cmd)
	{
		case APP_CMD_INIT_WINDOW:
		{
			window_event resizeEvent{window_event::Resize};
			resizeEvent.data.resize.w = cast<uint32_t>(ANativeWindow_getWidth(state->window));
			resizeEvent.data.resize.h = cast<uint32_t>(ANativeWindow_getHeight(state->window));

			wnd.queue_event(resizeEvent);
			wnd.queue_event({window_event::Restore});
			break;
		}

		case APP_CMD_TERM_WINDOW:
			wnd.queue_event({window_event::Suspend});
			break;

		case APP_CMD_GAINED_FOCUS:
			ASensorEventQueue_enableSensor(wnd.get_handle().sensorEventQueue, wnd.get_handle().accelerometerSensor);
			ASensorEventQueue_setEventRate(wnd.get_handle().sensorEventQueue, wnd.get_handle().accelerometerSensor, (1000 * 1000 / 60));
			wnd.queue_event({window_event::Focus});
			break;

		case APP_CMD_LOST_FOCUS:
			ASensorEventQueue_disableSensor(wnd.get_handle().sensorEventQueue, wnd.get_handle().accelerometerSensor);
			wnd.queue_event({window_event::Defocus});
			break;

		case APP_CMD_DESTROY:
			wnd.close();
			break;

		default:
			break;
	}
}

static int input_event(android_app* state, AInputEvent* e)
{
	switch (AInputEvent_getType(e))
	{
		default:
			break;
	}

	return 0;
}

window_handle create_window_handle(uint32_t width, uint32_t height)
{
	android_only::app->onInputEvent = &input_event;

	window_handle handle;
	handle.sensorManager = ASensorManager_getInstance();
	handle.accelerometerSensor = ASensorManager_getDefaultSensor(handle.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	handle.sensorEventQueue = ASensorManager_createEventQueue(handle.sensorManager, android_only::app->looper, LOOPER_ID_USER, nullptr, nullptr);

	/* Update until native window is initialized. */
	bool initialized = false;
	android_only::app->userData = &initialized;
	android_only::app->onAppCmd = [](android_app* state, int cmd)
	{
		if (cmd == APP_CMD_INIT_WINDOW)
			*cast<bool*>(state->userData) = true;
	};
	while (!initialized)
	{
		message msg;
		if (ALooper_pollAll(0, nullptr, &msg.events, (void**)&msg.source) >= 0)
		{
			if (msg.source)
				msg.source->process(android_only::app, msg.source);
		}
	}

	android_only::app->userData = nullptr;
	android_only::app->onAppCmd = &app_cmd;

	return handle;
}

void set_window_user_data(window_handle& /*wh*/, window& wnd)
{
	android_only::app->userData = &wnd;
}

std::optional<message> peek_message(const window_handle& wh)
{
	message msg{};
	if (ALooper_pollAll(0, nullptr, &msg.events, cast<void**>(&msg.source)) >= 0)
	{
		ASensorEventQueue_getEvents(wh.sensorEventQueue, &msg.sensorEvent, 1);
		return msg;
	}
	else
	{
		return std::nullopt;
	}
}

void process_message(window& /*wnd*/, const message& msg)
{
	if (msg.source)
		msg.source->process(android_only::app, msg.source);
}

void set_window_title(const window_handle& /*wh*/, const std::string& /*title*/)
{
	// #TODO: Activity.setTitle
}

void set_window_position(const window_handle& /*wh*/, int /*x*/, int /*y*/)
{
}

void set_window_size(const window_handle& /*wh*/, uint32_t /*width*/, uint32_t /*height*/)
{
}

void set_window_visibility(const window_handle& /*wh*/, bool /*visible*/)
{
	// #TODO: Activity.setVisible
}

}
}
