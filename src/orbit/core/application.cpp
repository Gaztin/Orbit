/*
 * Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "application.h"

#if defined( ORB_OS_IOS )
#  include <UIKit/UIKit.h>

@interface ORBAppDelegate : UIResponder< UIApplicationDelegate >
@property ( atomic ) std::shared_ptr< orb::application_base > app;
@end
#endif

namespace orb
{
	std::shared_ptr< void >( *application_base::s_initializer )() = nullptr;
	std::shared_ptr< application_base > application_base::s_instance;

	void application_base::run_instance()
	{
		if( s_instance || !s_initializer )
			return;

		/* Initialize application instance */
		s_instance = std::static_pointer_cast< application_base >( s_initializer() );

	#if defined( ORB_OS_IOS )

		@autoreleasepool
		{
			UIApplicationMain( 0, nil, nil, NSStringFromClass( [ ORBAppDelegate class ] ) );
		}

	#else

		while( s_instance->is_running() )
		{
			s_instance->frame();
		}

	#endif

	}
}

#if defined( ORB_OS_IOS )
#  include "orbit/core/log.h"

@implementation ORBAppDelegate

-( BOOL )application:( UIApplication* )application didFinishLaunchingWithOptions:( NSDictionary* )launchOptions
{
	orb::log_info( "didFinishLaunchingWithOptions()" );
	_app = application_base::s_instance;

	CADisplayLink* displayLink = [ CADisplayLink displayLinkWithTarget:application.delegate selector:@selector( frame: ) ];
	[ displayLink addToRunLoop:[ NSRunLoop currentRunLoop ] forMode:NSDefaultRunLoopMode ];

	return YES;
}

-( void )applicationWillResignActive:( UIApplication* )application
{
	orb::log_info( "applicationWillResignActive()" );
}

-( void )applicationDidEnterBackground:( UIApplication* )application
{
	orb::log_info( "applicationDidEnterBackground()" );
}

-( void )applicationWillEnterForeground:( UIApplication* )application
{
	orb::log_info( "applicationWillEnterForeground()" );
}

-( void )applicationDidBecomeActive:( UIApplication* )application
{
	orb::log_info( "applicationDidBecomeActive()" );
}

-( void )applicationWillTerminate:( UIApplication* )application
{
	orb::log_info( "applicationWillTerminate()" );
	_app->reset();
}

-( void )frame:( CADisplayLink* )displayLink
{
	_app->frame();
}

@end

#endif

