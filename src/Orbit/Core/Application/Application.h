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
#include <memory>
#include <type_traits>

#include "Orbit/Core/Core.h"

#define ORB_APP_DECL( APP_TYPE )                                                                       \
    class APP_TYPE;                                                                                    \
    volatile int _orb_app_initializer_eval = ORB_NAMESPACE Application< APP_TYPE >::_initializer_eval; \
    class APP_TYPE final : public ORB_NAMESPACE Application< APP_TYPE >

ORB_NAMESPACE_BEGIN

class ORB_API_CORE ApplicationBase
{
public:

	ApplicationBase( void ) = default;
	virtual ~ApplicationBase( void ) = default;

public:

	virtual void OnFrame( void ) { }
	virtual bool IsRunning( void ) = 0;

public:

	static void RunInstance( void );

};

extern ORB_API_CORE ApplicationBase*( *_application_initializer )( void );

template< typename Derived >
class Application : private ApplicationBase
{
public:

	virtual ~Application( void ) = default;

public:

	static volatile int _initializer_eval;

};

template< typename Derived >
volatile int Application< Derived >::_initializer_eval = ( _application_initializer = []( void ) -> ApplicationBase* { return new Derived(); }, 1 );

ORB_NAMESPACE_END
