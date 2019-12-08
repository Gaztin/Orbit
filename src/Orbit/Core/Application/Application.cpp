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

#include <chrono>

#include "Orbit/Core/Platform/iOS/UIApplicationDelegate.h"

ORB_NAMESPACE_BEGIN

std::shared_ptr< void >( *_application_initializer )( void );

void ApplicationBase::RunInstance()
{

#if defined( ORB_OS_IOS )

	@autoreleasepool
	{
		UIApplicationMain( 0, nil, nil, NSStringFromClass( [ ORB_NAMESPACED_OBJC( UIApplicationDelegate ) class ] ) );
	}

#else

	if( !_application_initializer )
		return;

	/* Initialize application instance */
	auto instance = std::static_pointer_cast< ApplicationBase >( _application_initializer() );
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
