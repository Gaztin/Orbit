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

#include "Application.h"

#if defined( ORB_OS_IOS )
#  include <UIKit/UIKit.h>
@interface OrbitApplicationDelegate : UIResponder< UIApplicationDelegate >
@end
#endif

ORB_NAMESPACE_BEGIN

ApplicationBase*( *_application_initializer )() = nullptr;

void ApplicationBase::RunInstance()
{
#if defined( ORB_OS_IOS )

	@autoreleasepool
	{
		UIApplicationMain( 0, nil, nil, NSStringFromClass( [ OrbitApplicationDelegate class ] ) );
	}

#else

	if( !_application_initializer )
		return;

	/* Initialize application instance */
	ApplicationBase* instance = _application_initializer();

	while( instance->IsRunning() )
	{
		instance->OnFrame();
	}

	delete instance;

#endif
}

ORB_NAMESPACE_END

#if defined( ORB_OS_IOS )
#  include "Orbit/Core/Log.h"

@implementation OrbitApplicationDelegate

-( BOOL )application:( UIApplication* )__unused application didFinishLaunchingWithOptions:( NSDictionary* )__unused launchOptions
{
	ORB_NAMESPACE log_info( "didFinishLaunchingWithOptions()" );

	if( ORB_NAMESPACE _application_initializer && !ORB_NAMESPACE _application_instance )
		ORB_NAMESPACE _application_instance = std::static_pointer_cast< ORB_NAMESPACE ApplicationBase >( ORB_NAMESPACE _application_initializer() );

	CADisplayLink* display_link = [ CADisplayLink displayLinkWithTarget:application.delegate selector:@selector( frame: ) ];
	[ display_link addToRunLoop:[ NSRunLoop currentRunLoop ] forMode:NSDefaultRunLoopMode ];

	return YES;
}

-( void )applicationWillResignActive:( UIApplication* )__unused application
{
	ORB_NAMESPACE log_info( "applicationWillResignActive()" );
}

-( void )applicationDidEnterBackground:( UIApplication* )__unused application
{
	ORB_NAMESPACE log_info( "applicationDidEnterBackground()" );
}

-( void )applicationWillEnterForeground:( UIApplication* )__unused application
{
	ORB_NAMESPACE log_info( "applicationWillEnterForeground()" );
}

-( void )applicationDidBecomeActive:( UIApplication* )__unused application
{
	ORB_NAMESPACE log_info( "applicationDidBecomeActive()" );
}

-( void )applicationWillTerminate:( UIApplication* )__unused application
{
	ORB_NAMESPACE log_info( "applicationWillTerminate()" );
	ORB_NAMESPACE _application_instance.reset();
}

-( void )frame:( CADisplayLink* )__unused displayLink
{
	if( ORB_NAMESPACE _application_instance )
		ORB_NAMESPACE _application_instance->OnFrame();
}

@end

#endif

