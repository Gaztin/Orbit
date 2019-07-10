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
  #include <UIKit/UIKit.h>
@interface ORBAppDelegate : UIResponder< UIApplicationDelegate >
@end
#endif

namespace orb
{
	std::shared_ptr< void >( *__application_initializer )() = nullptr;
	std::shared_ptr< application_base > __application_instance;

	void application_base::run_instance()
	{
	#if defined( ORB_OS_IOS )

		@autoreleasepool
		{
			UIApplicationMain( 0, nil, nil, NSStringFromClass( [ ORBAppDelegate class ] ) );
		}

	#else

		if( s_instance || !__application_initializer )
			return;

		/* Initialize application instance */
		__application_instance = std::static_pointer_cast< application_base >( __application_initializer() );

		while( __application_instance->is_running() )
		{
			__application_instance->frame();
		}

	#endif

	}
}

#if defined( ORB_OS_IOS )
#  include "orbit/core/log.h"

@implementation ORBAppDelegate

-( BOOL )application:( UIApplication* )__unused application didFinishLaunchingWithOptions:( NSDictionary* )__unused launchOptions
{
	orb::log_info( "didFinishLaunchingWithOptions()" );

	if( orb::__application_initializer && !orb::__application_instance )
		orb::__application_instance = std::static_pointer_cast< orb::application_base >( orb::__application_initializer() );

	CADisplayLink* displayLink = [ CADisplayLink displayLinkWithTarget:application.delegate selector:@selector( frame: ) ];
	[ displayLink addToRunLoop:[ NSRunLoop currentRunLoop ] forMode:NSDefaultRunLoopMode ];

	return YES;
}

-( void )applicationWillResignActive:( UIApplication* )__unused application
{
	orb::log_info( "applicationWillResignActive()" );
}

-( void )applicationDidEnterBackground:( UIApplication* )__unused application
{
	orb::log_info( "applicationDidEnterBackground()" );
}

-( void )applicationWillEnterForeground:( UIApplication* )__unused application
{
	orb::log_info( "applicationWillEnterForeground()" );
}

-( void )applicationDidBecomeActive:( UIApplication* )__unused application
{
	orb::log_info( "applicationDidBecomeActive()" );
}

-( void )applicationWillTerminate:( UIApplication* )__unused application
{
	orb::log_info( "applicationWillTerminate()" );
	orb::__application_instance.reset();
}

-( void )frame:( CADisplayLink* )__unused displayLink
{
	if( orb::__application_instance )
		orb::__application_instance->frame();
}

@end

#endif

