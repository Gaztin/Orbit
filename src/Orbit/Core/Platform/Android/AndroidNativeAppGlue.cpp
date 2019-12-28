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

#include "AndroidNativeAppGlue.h"

#if defined( ORB_OS_ANDROID )
#  include <cerrno>
#  include <cstdlib>
#  include <cstring>

#  include <android/log.h>
#  include <sys/resource.h>
#  include <jni.h>
#  include <unistd.h>

#  include "Orbit/Core/IO/Log.h" 

ORB_NAMESPACE_BEGIN

static void FreeSavedState( AndroidApp* android_app )
{
	android_app->mutex.lock();
	if( android_app->saved_state != NULL )
	{
		free( android_app->saved_state );
		android_app->saved_state = NULL;
		android_app->saved_state_size = 0;
	}
	android_app->mutex.unlock();
}

static AndroidAppCommand AndroidAppReadCommand( AndroidApp* android_app )
{
	AndroidAppCommand cmd;

	if( read( android_app->msgread, &cmd, sizeof( cmd ) ) == sizeof( cmd ) )
	{
		switch( cmd )
		{
			default: break;

			case AndroidAppCommand::SaveState:
				FreeSavedState( android_app );
				break;
		}
		return cmd;
	} else
	{
		LogError( "No data on command pipe!" );
	}
	return static_cast< AndroidAppCommand >( -1 );
}

static void PrintCurrentConfig( AndroidApp* android_app )
{
	char lang[2];
	char country[2];
	AConfiguration_getLanguage( android_app->config, lang );
	AConfiguration_getCountry( android_app->config, country );

	LogInfo( "Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
		  "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
		  "modetype=%d modenight=%d", AConfiguration_getMcc( android_app->config ), AConfiguration_getMnc( android_app->config ), lang[ 0 ], lang[ 1 ], country[ 0 ], country[ 1 ], AConfiguration_getOrientation( android_app->config ), AConfiguration_getTouchscreen( android_app->config ), AConfiguration_getDensity( android_app->config ), AConfiguration_getKeyboard( android_app->config ), AConfiguration_getNavigation( android_app->config ), AConfiguration_getKeysHidden( android_app->config ), AConfiguration_getNavHidden( android_app->config ), AConfiguration_getSdkVersion( android_app->config ), AConfiguration_getScreenSize( android_app->config ), AConfiguration_getScreenLong( android_app->config ), AConfiguration_getUiModeType( android_app->config ), AConfiguration_getUiModeNight( android_app->config ) );
}

static void AndroidAppPreExecCommand( AndroidApp* android_app, AndroidAppCommand cmd )
{
	switch( cmd )
	{
		default: break;

		case AndroidAppCommand::InputChanged:
			LogInfo( "APP_CMD_INPUT_CHANGED\n" );
			android_app->mutex.lock();
			if( android_app->input_queue != NULL )
			{
				AInputQueue_detachLooper( android_app->input_queue );
			}
			android_app->input_queue = android_app->pending_input_queue;
			if( android_app->input_queue != NULL )
			{
				LogInfo( "Attaching input queue to looper" );
				AInputQueue_attachLooper( android_app->input_queue, android_app->looper, static_cast< int >( AndroidLooperID::Input ), NULL, &android_app->input_poll_source );
			}
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;

		case AndroidAppCommand::InitWindow:
			LogInfo( "APP_CMD_INIT_WINDOW\n" );
			android_app->mutex.lock();
			android_app->window = android_app->pending_window;
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;

		case AndroidAppCommand::TermWindow:
			LogInfo( "APP_CMD_TERM_WINDOW\n" );
			android_app->cond.notify_all();
			break;

		case AndroidAppCommand::Resume:
		case AndroidAppCommand::Start:
		case AndroidAppCommand::Pause:
		case AndroidAppCommand::Stop:
			LogInfo( "activity_state=%d\n", static_cast< int >( cmd ) );
			android_app->mutex.lock();
			android_app->activity_state = cmd;
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;

		case AndroidAppCommand::ConfigChanged:
			LogInfo( "APP_CMD_CONFIG_CHANGED\n" );
			AConfiguration_fromAssetManager( android_app->config, android_app->activity->assetManager );
			PrintCurrentConfig( android_app );
			break;

		case AndroidAppCommand::Destroy:
			LogInfo( "APP_CMD_DESTROY\n" );
			android_app->destroy_requested = 1;
			break;
	}
}

static void AndroidAppPostExecCommand( AndroidApp* android_app, AndroidAppCommand cmd )
{
	switch( cmd )
	{
		default: break;

		case AndroidAppCommand::TermWindow:
			LogInfo( "APP_CMD_TERM_WINDOW\n" );
			android_app->mutex.lock();
			android_app->window = NULL;
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;

		case AndroidAppCommand::SaveState:
			LogInfo( "APP_CMD_SAVE_STATE\n" );
			android_app->mutex.lock();
			android_app->state_saved = 1;
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;

		case AndroidAppCommand::Resume:
			FreeSavedState( android_app );
			break;
	}
}

static void AndroidAppDestroy( AndroidApp* android_app )
{
	LogInfo( "android_app_destroy!" );
	FreeSavedState( android_app );
	android_app->mutex.lock();
	if( android_app->input_queue != NULL )
	{
		AInputQueue_detachLooper( android_app->input_queue );
	}
	AConfiguration_delete( android_app->config );
	android_app->destroyed = 1;
	android_app->cond.notify_all();
	android_app->mutex.unlock();
	// Can't touch android_app object after this.
}

static void ProcessInput( AndroidApp* app, AndroidPollSource* /*source*/ )
{
	AInputEvent* event = NULL;
	while( AInputQueue_getEvent( app->input_queue, &event ) >= 0 )
	{
		LogInfo( "New input event: type=%d\n", AInputEvent_getType( event ) );
		if( AInputQueue_preDispatchEvent( app->input_queue, event ) )
		{
			continue;
		}
		int32_t handled = 0;
		if( app->on_input_event != NULL ) handled = app->on_input_event( app, event );
		AInputQueue_finishEvent( app->input_queue, event, handled );
	}
}

static void ProcessCommand( AndroidApp* app, AndroidPollSource* /*source*/ )
{
	AndroidAppCommand cmd = AndroidAppReadCommand( app );
	AndroidAppPreExecCommand( app, cmd );
	if( app->on_app_cmd != NULL ) app->on_app_cmd( app, cmd );
	AndroidAppPostExecCommand( app, cmd );
}

static void AndroidAppEntry( AndroidApp* android_app )
{
	android_app->config = AConfiguration_new();
	AConfiguration_fromAssetManager( android_app->config, android_app->activity->assetManager );

	PrintCurrentConfig( android_app );

	android_app->cmd_poll_source.id = AndroidLooperID::Main;
	android_app->cmd_poll_source.app = android_app;
	android_app->cmd_poll_source.process = ProcessCommand;
	android_app->input_poll_source.id = AndroidLooperID::Input;
	android_app->input_poll_source.app = android_app;
	android_app->input_poll_source.process = ProcessInput;

	ALooper* looper = ALooper_prepare( ALOOPER_PREPARE_ALLOW_NON_CALLBACKS );
	ALooper_addFd( looper, android_app->msgread, static_cast< int >( AndroidLooperID::Main ), ALOOPER_EVENT_INPUT, NULL, &android_app->cmd_poll_source );
	android_app->looper = looper;

	android_app->mutex.lock();
	android_app->running = 1;
	android_app->cond.notify_all();
	android_app->mutex.unlock();

	AndroidMain( android_app );

	AndroidAppDestroy( android_app );
}

static void AndroidAppWriteCommand( AndroidApp* android_app, AndroidAppCommand cmd )
{
	if( write( android_app->msgwrite, &cmd, sizeof( cmd ) ) != sizeof( cmd ) )
	{
		LogError( "Failure writing android_app cmd: %s\n", strerror( errno ) );
	}
}

static void AndroidAppSetActivityState( AndroidApp* android_app, AndroidAppCommand cmd )
{
	std::unique_lock lock( android_app->mutex );
	AndroidAppWriteCommand( android_app, cmd );
	while( android_app->activity_state != cmd )
	{
		android_app->cond.wait( lock );
	}
}

static void AndroidAppSetWindow( AndroidApp* android_app, ANativeWindow* window )
{
	std::unique_lock lock( android_app->mutex );
	if( android_app->pending_window != NULL )
	{
		AndroidAppWriteCommand( android_app, AndroidAppCommand::TermWindow );
	}
	android_app->pending_window = window;
	if( window != NULL )
	{
		AndroidAppWriteCommand( android_app, AndroidAppCommand::InitWindow );
	}
	while( android_app->window != android_app->pending_window )
	{
		android_app->cond.wait( lock );
	}
}

static void AndroidAppSetInput( AndroidApp* android_app, AInputQueue* input_queue )
{
	std::unique_lock lock( android_app->mutex );
	android_app->pending_input_queue = input_queue;
	AndroidAppWriteCommand( android_app, AndroidAppCommand::InputChanged );
	while( android_app->input_queue != android_app->pending_input_queue )
	{
		android_app->cond.wait( lock );
	}
}

static void AndroidAppFree( AndroidApp* android_app )
{
	std::unique_lock lock( android_app->mutex );
	AndroidAppWriteCommand( android_app, AndroidAppCommand::Destroy );
	while( !android_app->destroyed )
	{
		android_app->cond.wait( lock );
	}

	close( android_app->msgread );
	close( android_app->msgwrite );

	delete android_app;
}

static void OnDestroy( ANativeActivity* activity )
{
	LogInfo( "Destroy: %p\n", activity );
	AndroidAppFree( ( AndroidApp* )activity->instance );
}

static void OnStart( ANativeActivity* activity )
{
	LogInfo( "Start: %p\n", activity );
	AndroidAppSetActivityState( ( AndroidApp* )activity->instance, AndroidAppCommand::Start );
}

static void OnResume( ANativeActivity* activity )
{
	LogInfo( "Resume: %p\n", activity );
	AndroidAppSetActivityState( ( AndroidApp* )activity->instance, AndroidAppCommand::Resume );
}

static void* OnSaveInstanceState( ANativeActivity* activity, size_t* outLen )
{
	AndroidApp* android_app = ( AndroidApp* )activity->instance;
	void* saved_state = NULL;

	LogInfo( "SaveInstanceState: %p\n", activity );
	std::unique_lock lock( android_app->mutex );
	android_app->state_saved = 0;
	AndroidAppWriteCommand( android_app, AndroidAppCommand::SaveState );
	while( !android_app->state_saved )
	{
		android_app->cond.wait( lock );
	}

	if( android_app->saved_state != NULL )
	{
		saved_state = android_app->saved_state;
		*outLen = android_app->saved_state_size;
		android_app->saved_state = NULL;
		android_app->saved_state_size = 0;
	}

	return saved_state;
}

static void OnPause( ANativeActivity* activity )
{
	LogInfo( "Pause: %p\n", activity );
	AndroidAppSetActivityState( ( AndroidApp* )activity->instance, AndroidAppCommand::Pause );
}

static void OnStop( ANativeActivity* activity )
{
	LogInfo( "Stop: %p\n", activity );
	AndroidAppSetActivityState( ( AndroidApp* )activity->instance, AndroidAppCommand::Stop );
}

static void OnConfigurationChanged( ANativeActivity* activity )
{
	AndroidApp* android_app = ( AndroidApp* )activity->instance;
	LogInfo( "ConfigurationChanged: %p\n", activity );
	AndroidAppWriteCommand( android_app, AndroidAppCommand::ConfigChanged );
}

static void OnLowMemory( ANativeActivity* activity )
{
	AndroidApp* android_app = ( AndroidApp* )activity->instance;
	LogInfo( "LowMemory: %p\n", activity );
	AndroidAppWriteCommand( android_app, AndroidAppCommand::LowMemory );
}

static void OnWindowFocusChanged( ANativeActivity* activity, int focused )
{
	LogInfo( "WindowFocusChanged: %p -- %d\n", activity, focused );
	AndroidAppWriteCommand( ( AndroidApp* )activity->instance, focused
															   ? AndroidAppCommand::GainedFocus
															   : AndroidAppCommand::LostFocus );
}

static void OnNativeWindowCreated( ANativeActivity* activity, ANativeWindow* window )
{
	LogInfo( "NativeWindowCreated: %p -- %p\n", activity, window );
	AndroidAppSetWindow( ( AndroidApp* )activity->instance, window );
}

static void OnNativeWindowDestroyed( ANativeActivity* activity, ANativeWindow* window )
{
	LogInfo( "NativeWindowDestroyed: %p -- %p\n", activity, window );
	AndroidAppSetWindow( ( AndroidApp* )activity->instance, NULL );
}

static void OnInputQueueCreated( ANativeActivity* activity, AInputQueue* queue )
{
	LogInfo( "InputQueueCreated: %p -- %p\n", activity, queue );
	AndroidAppSetInput( ( AndroidApp* )activity->instance, queue );
}

static void OnInputQueueDestroyed( ANativeActivity* activity, AInputQueue* queue )
{
	LogInfo( "InputQueueDestroyed: %p -- %p\n", activity, queue );
	AndroidAppSetInput( ( AndroidApp* ) activity->instance, NULL );
}

AndroidApp* AndroidAppCreate( ANativeActivity* activity, void* saved_state, size_t saved_state_size )
{
	AndroidApp* android_app = new AndroidApp { };
	android_app->activity = activity;

	if( saved_state != NULL )
	{
		android_app->saved_state = malloc( saved_state_size );
		android_app->saved_state_size = saved_state_size;
		memcpy( android_app->saved_state, saved_state, saved_state_size );
	}

	int msgpipe[ 2 ];
	if( pipe( msgpipe ) )
	{
		LogError( "could not create pipe: %s", strerror( errno ) );
		return NULL;
	}
	android_app->msgread = msgpipe[ 0 ];
	android_app->msgwrite = msgpipe[ 1 ];

	android_app->thread = std::thread( AndroidAppEntry, android_app );
	android_app->thread.detach();

	// Wait for thread to start.
	{
		std::unique_lock lock( android_app->mutex );
		while( !android_app->running )
		{
			android_app->cond.wait( lock );
		}
	}

	activity->callbacks->onDestroy               = OnDestroy;
	activity->callbacks->onStart                 = OnStart;
	activity->callbacks->onResume                = OnResume;
	activity->callbacks->onSaveInstanceState     = OnSaveInstanceState;
	activity->callbacks->onPause                 = OnPause;
	activity->callbacks->onStop                  = OnStop;
	activity->callbacks->onConfigurationChanged  = OnConfigurationChanged;
	activity->callbacks->onLowMemory             = OnLowMemory;
	activity->callbacks->onWindowFocusChanged    = OnWindowFocusChanged;
	activity->callbacks->onNativeWindowCreated   = OnNativeWindowCreated;
	activity->callbacks->onNativeWindowDestroyed = OnNativeWindowDestroyed;
	activity->callbacks->onInputQueueCreated     = OnInputQueueCreated;
	activity->callbacks->onInputQueueDestroyed   = OnInputQueueDestroyed;

	return android_app;
}

ORB_NAMESPACE_END

#endif
