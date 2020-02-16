/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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
#  include "Orbit/Core/IO/Pipe.h"

#  include <condition_variable>
#  include <mutex>
#  include <thread>

#  include <android/configuration.h>
#  include <android/looper.h>
#  include <android/native_activity.h>

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

	std::unique_ptr< uint8_t[] > saved_state;
	std::mutex                   mutex;
	std::condition_variable      cond;
	std::thread                  thread;

	Pipe              pipe;
	AndroidAppCommand activity_state;
	AndroidPollSource cmd_poll_source;
	AndroidPollSource input_poll_source;

	ANativeActivity* activity;
	AConfiguration*  config;
	ALooper*         looper;
	AInputQueue*     input_queue;
	ANativeWindow*   window;
	AInputQueue*     pending_input_queue;
	ANativeWindow*   pending_window;

	size_t saved_state_size;

	int destroy_requested;
	int running;
	int state_saved;
	int destroyed;
};

extern ORB_API_CORE AndroidApp* AndroidAppCreate( ANativeActivity* activity, void* saved_state, size_t saved_state_size );

extern void AndroidMain( AndroidApp* app );

ORB_NAMESPACE_END

#endif // ORB_OS_ANDROID
