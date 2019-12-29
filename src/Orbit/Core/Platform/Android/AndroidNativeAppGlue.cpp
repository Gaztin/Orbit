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

#  include <unistd.h>

#  include "Orbit/Core/IO/Log.h"

#  pragma clang diagnostic push
#  pragma ide diagnostic ignored "hicpp-use-auto"

ORB_NAMESPACE_BEGIN

static void FreeSavedState( AndroidApp* android_app )
{
	std::unique_lock lock( android_app->mutex );
	android_app->saved_state.reset();
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
	}
	else
	{
		LogError( "No data on command pipe!" );
		return static_cast< AndroidAppCommand >( -1 );
	}
}

static void PrintCurrentConfig( AndroidApp* android_app )
{
	char lang[ 2 ];
	char country[ 2 ];

	AConfiguration_getLanguage( android_app->config, lang );
	AConfiguration_getCountry( android_app->config, country );
	LogInfo( "Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d modetype=%d modenight=%d", AConfiguration_getMcc( android_app->config ), AConfiguration_getMnc( android_app->config ), lang[ 0 ], lang[ 1 ], country[ 0 ], country[ 1 ], AConfiguration_getOrientation( android_app->config ), AConfiguration_getTouchscreen( android_app->config ), AConfiguration_getDensity( android_app->config ), AConfiguration_getKeyboard( android_app->config ), AConfiguration_getNavigation( android_app->config ), AConfiguration_getKeysHidden( android_app->config ), AConfiguration_getNavHidden( android_app->config ), AConfiguration_getSdkVersion( android_app->config ), AConfiguration_getScreenSize( android_app->config ), AConfiguration_getScreenLong( android_app->config ), AConfiguration_getUiModeType( android_app->config ), AConfiguration_getUiModeNight( android_app->config ) );
}

static void AndroidAppPreExecCommand( AndroidApp* android_app, AndroidAppCommand cmd )
{
	switch( cmd )
	{
		default: break;

		case AndroidAppCommand::InputChanged:
		{
			android_app->mutex.lock();
			if( android_app->input_queue != nullptr )
			{
				AInputQueue_detachLooper( android_app->input_queue );
			}
			android_app->input_queue = android_app->pending_input_queue;
			if( android_app->input_queue != nullptr )
			{
				AInputQueue_attachLooper( android_app->input_queue, android_app->looper, static_cast< int >( AndroidLooperID::Input ), nullptr, &android_app->input_poll_source );
			}
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;
		}

		case AndroidAppCommand::InitWindow:
		{
			android_app->mutex.lock();
			android_app->window = android_app->pending_window;
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;
		}

		case AndroidAppCommand::TermWindow:
		{
			android_app->cond.notify_all();
			break;
		}

		case AndroidAppCommand::Resume:
		case AndroidAppCommand::Start:
		case AndroidAppCommand::Pause:
		case AndroidAppCommand::Stop:
		{
			android_app->mutex.lock();
			android_app->activity_state = cmd;
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;
		}

		case AndroidAppCommand::ConfigChanged:
		{
			AConfiguration_fromAssetManager( android_app->config, android_app->activity->assetManager );
			PrintCurrentConfig( android_app );
			break;
		}

		case AndroidAppCommand::Destroy:
		{
			android_app->destroy_requested = 1;
			break;
		}
	}
}

static void AndroidAppPostExecCommand( AndroidApp* android_app, AndroidAppCommand cmd )
{
	switch( cmd )
	{
		default: break;

		case AndroidAppCommand::TermWindow:
		{
			android_app->mutex.lock();
			android_app->window = nullptr;
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;
		}

		case AndroidAppCommand::SaveState:
		{
			android_app->mutex.lock();
			android_app->state_saved = 1;
			android_app->cond.notify_all();
			android_app->mutex.unlock();
			break;
		}

		case AndroidAppCommand::Resume:
		{
			FreeSavedState( android_app );
			break;
		}
	}
}

static void AndroidAppDestroy( AndroidApp* android_app )
{
	FreeSavedState( android_app );

	std::unique_lock lock( android_app->mutex );

	if( android_app->input_queue != nullptr )
	{
		AInputQueue_detachLooper( android_app->input_queue );
	}

	AConfiguration_delete( android_app->config );

	android_app->destroyed = 1;
	android_app->cond.notify_all();
}

static void ProcessInput( AndroidApp* app, AndroidPollSource* /*source*/ )
{
	AInputEvent* event = nullptr;

	while( AInputQueue_getEvent( app->input_queue, &event ) >= 0 )
	{
		if( AInputQueue_preDispatchEvent( app->input_queue, event ) )
		{
			continue;
		}

		int32_t handled = 0;
		if( app->on_input_event != nullptr )
		{
			handled = app->on_input_event( app, event );
		}

		AInputQueue_finishEvent( app->input_queue, event, handled );
	}
}

static void ProcessCommand( AndroidApp* app, AndroidPollSource* /*source*/ )
{
	AndroidAppCommand cmd = AndroidAppReadCommand( app );

	AndroidAppPreExecCommand( app, cmd );

	if( app->on_app_cmd != nullptr )
	{
		app->on_app_cmd( app, cmd );
	}

	AndroidAppPostExecCommand( app, cmd );
}

static void AndroidAppEntry( AndroidApp* android_app )
{
	android_app->config = AConfiguration_new();
	AConfiguration_fromAssetManager( android_app->config, android_app->activity->assetManager );

	PrintCurrentConfig( android_app );

	android_app->cmd_poll_source.id        = AndroidLooperID::Main;
	android_app->cmd_poll_source.app       = android_app;
	android_app->cmd_poll_source.process   = ProcessCommand;
	android_app->input_poll_source.id      = AndroidLooperID::Input;
	android_app->input_poll_source.app     = android_app;
	android_app->input_poll_source.process = ProcessInput;

	ALooper* looper = ALooper_prepare( ALOOPER_PREPARE_ALLOW_NON_CALLBACKS );
	ALooper_addFd( looper, android_app->msgread, static_cast< int >( AndroidLooperID::Main ), ALOOPER_EVENT_INPUT, nullptr, &android_app->cmd_poll_source );
	android_app->looper = looper;

	{
		std::unique_lock lock( android_app->mutex );
		android_app->running = 1;
		android_app->cond.notify_all();
	}

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

	if( android_app->pending_window != nullptr )
	{
		AndroidAppWriteCommand( android_app, AndroidAppCommand::TermWindow );
	}

	android_app->pending_window = window;

	if( window != nullptr )
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
	AndroidAppFree( static_cast< AndroidApp* >( activity->instance ) );
}

static void OnStart( ANativeActivity* activity )
{
	AndroidAppSetActivityState( static_cast< AndroidApp* >( activity->instance ), AndroidAppCommand::Start );
}

static void OnResume( ANativeActivity* activity )
{
	AndroidAppSetActivityState( static_cast< AndroidApp* >( activity->instance ), AndroidAppCommand::Resume );
}

static void* OnSaveInstanceState( ANativeActivity* activity, size_t* outLen )
{
	AndroidApp*      android_app = static_cast< AndroidApp* >( activity->instance );
	std::unique_lock lock( android_app->mutex );

	android_app->state_saved = 0;

	AndroidAppWriteCommand( android_app, AndroidAppCommand::SaveState );

	while( !android_app->state_saved )
	{
		android_app->cond.wait( lock );
	}

	if( android_app->saved_state )
	{
		*outLen = android_app->saved_state_size;
		return android_app->saved_state.release();
	}

	return nullptr;
}

static void OnPause( ANativeActivity* activity )
{
	AndroidAppSetActivityState( static_cast< AndroidApp* >( activity->instance ), AndroidAppCommand::Pause );
}

static void OnStop( ANativeActivity* activity )
{
	AndroidAppSetActivityState( static_cast< AndroidApp* >( activity->instance ), AndroidAppCommand::Stop );
}

static void OnConfigurationChanged( ANativeActivity* activity )
{
	AndroidApp* android_app = static_cast< AndroidApp* >( activity->instance );
	AndroidAppWriteCommand( android_app, AndroidAppCommand::ConfigChanged );
}

static void OnLowMemory( ANativeActivity* activity )
{
	AndroidApp* android_app = static_cast< AndroidApp* >( activity->instance );
	AndroidAppWriteCommand( android_app, AndroidAppCommand::LowMemory );
}

static void OnWindowFocusChanged( ANativeActivity* activity, int focused )
{
	AndroidAppWriteCommand( static_cast< AndroidApp* >( activity->instance ), focused ? AndroidAppCommand::GainedFocus : AndroidAppCommand::LostFocus );
}

static void OnNativeWindowCreated( ANativeActivity* activity, ANativeWindow* window )
{
	AndroidAppSetWindow( static_cast< AndroidApp* >( activity->instance ), window );
}

static void OnNativeWindowDestroyed( ANativeActivity* activity, ANativeWindow* /*window*/ )
{
	AndroidAppSetWindow( static_cast< AndroidApp* >( activity->instance ), nullptr );
}

static void OnInputQueueCreated( ANativeActivity* activity, AInputQueue* queue )
{
	AndroidAppSetInput( static_cast< AndroidApp* >( activity->instance ), queue );
}

static void OnInputQueueDestroyed( ANativeActivity* activity, AInputQueue* /*queue*/ )
{
	AndroidAppSetInput( static_cast< AndroidApp* >( activity->instance ), nullptr );
}

AndroidApp* AndroidAppCreate( ANativeActivity* activity, void* saved_state, size_t saved_state_size )
{
	AndroidApp* android_app = new AndroidApp { };
	android_app->activity = activity;

	if( saved_state != nullptr )
	{
		android_app->saved_state.reset( new uint8_t[ saved_state_size ] );
		android_app->saved_state_size = saved_state_size;
		memcpy( &android_app->saved_state[ 0 ], saved_state, saved_state_size );
	}

	int msgpipe[ 2 ];
	if( pipe( msgpipe ) )
	{
		LogError( "could not create pipe: %s", strerror( errno ) );
		return nullptr;
	}
	android_app->msgread  = msgpipe[ 0 ];
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

#pragma clang diagnostic pop

#endif
