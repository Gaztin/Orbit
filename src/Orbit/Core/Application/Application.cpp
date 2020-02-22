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

#include "Application.h"

#include "Orbit/Core/Platform/iOS/UIApplicationDelegate.h"
#include "Orbit/Core/Widget/Console.h"

#include <chrono>

ORB_NAMESPACE_BEGIN

void ApplicationBase::RunInstance( void )
{
	Console console;

#if defined( ORB_OS_IOS )

	@autoreleasepool
	{
		UIApplicationMain( 0, nil, nil, NSStringFromClass( [ ORB_NAMESPACED_OBJC( UIApplicationDelegate ) class ] ) );
	}

#else

	if( !Bootstrap::trampoline )
		return;

	/* Initialize application instance */
	auto instance = std::static_pointer_cast< ApplicationBase >( Bootstrap::trampoline() );
	auto time     = std::chrono::high_resolution_clock::now();

	while( instance->IsRunning() )
	{
		auto now   = std::chrono::high_resolution_clock::now();
		auto delta = std::chrono::duration_cast< std::chrono::duration< float > >( now - time );

		time = now;

		instance->OnFrame( delta.count() );
	}

#endif

}

ORB_NAMESPACE_END
