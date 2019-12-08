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

#include "UIApplicationDelegate.h"

#if defined( ORB_OS_IOS )

@implementation ORB_NAMESPACED_OBJC( UIApplicationDelegate )

-( BOOL )application:( UIApplication* )__unused application didFinishLaunchingWithOptions:( NSDictionary* )__unused launchOptions
{
	_application_instance = std::static_pointer_cast< ORB_NAMESPACE ApplicationBase >( ORB_NAMESPACE _application_initializer() );

	CADisplayLink* display_link = [ CADisplayLink displayLinkWithTarget:application.delegate selector:@selector( OnFrame: ) ];
	[ display_link addToRunLoop:[ NSRunLoop currentRunLoop ] forMode:NSDefaultRunLoopMode ];

	return YES;
}

-( void )applicationWillResignActive:( UIApplication* )__unused application
{
}

-( void )applicationDidEnterBackground:( UIApplication* )__unused application
{
}

-( void )applicationWillEnterForeground:( UIApplication* )__unused application
{
}

-( void )applicationDidBecomeActive:( UIApplication* )__unused application
{
}

-( void )applicationWillTerminate:( UIApplication* )__unused application
{
}

-( void )OnFrame:( CADisplayLink* )display_link
{
	float delta_time = static_cast< float >( [ display_link duration ] );

	_application_instance->OnFrame( delta_time );
}

@end

#endif
