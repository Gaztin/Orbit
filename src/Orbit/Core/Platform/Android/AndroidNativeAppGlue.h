/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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
#include "Orbit/Core/Core.h"

#if defined( ORB_OS_ANDROID )
#  include <condition_variable>
#  include <mutex>
#  include <thread>

#  include <android/configuration.h>
#  include <android/looper.h>
#  include <android/native_activity.h>
#  include <poll.h>
#  include <pthread.h>
#  include <sched.h>

ORB_NAMESPACE_BEGIN

struct AndroidApp;

enum class AndroidLooperID : int32_t
{
	Main  = 1,
	Input = 2,
	User  = 3,
};

enum class AndroidAppCommand : int8_t
{
	InputChanged,
	InitWindow,
	TermWindow,
	WindowResized,
	RedrawNeeded,
	RectChanged,
	GainedFocus,
	LostFocus,
	ConfigChanged,
	LowMemory,
	Start,
	Resume,
	SaveState,
	Pause,
	Stop,
	Destroy,
};

struct ORB_API_CORE AndroidPollSource
{
	AndroidLooperID id;
	AndroidApp*     app;

	void ( *process )( AndroidApp* app, AndroidPollSource* source );
};

struct ORB_API_CORE AndroidApp
{
	void* user_data;

	void    ( *on_app_cmd     )( AndroidApp* app, AndroidAppCommand cmd );
	int32_t ( *on_input_event )( AndroidApp* app, AInputEvent* event );

	ANativeActivity*        activity;
	AConfiguration*         config;
	void*                   saved_state;
	size_t                  saved_state_size;
	ALooper*                looper;
	AInputQueue*            input_queue;
	ANativeWindow*          window;
	ARect                   content_rect;
	AndroidAppCommand       activity_state;
	int                     destroy_requested;
	std::mutex              mutex;
	std::condition_variable cond;
	int                     msgread;
	int                     msgwrite;
	std::thread             thread;
	AndroidPollSource       cmd_poll_source;
	AndroidPollSource       input_poll_source;
	int                     running;
	int                     state_saved;
	int                     destroyed;
	int                     redraw_needed;
	AInputQueue*            pending_input_queue;
	ANativeWindow*          pending_window;
	ARect                   pending_content_rect;
};

extern void AndroidMain( AndroidApp* app );

ORB_NAMESPACE_END

#endif
