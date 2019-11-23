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

#include "Orbit/Core.h"

#define ORB_APP_DECL( APP_TYPE )                                                                       \
    class APP_TYPE; /* Forward declaration */                                                          \
    volatile int _orb_app_initializer_eval = ORB_NAMESPACE Application< APP_TYPE >::_initializer_eval; \
    class APP_TYPE final : public ORB_NAMESPACE Application< APP_TYPE >

ORB_NAMESPACE_BEGIN

class ORB_API_CORE ApplicationBase
{
public:
	ApplicationBase() = default;
	virtual ~ApplicationBase() = default;

	virtual void OnFrame() { }
	virtual bool IsRunning() = 0;

	static void RunInstance();
};

extern ORB_API_CORE std::shared_ptr< void >( *_application_initializer )();
extern ORB_API_CORE std::shared_ptr< ApplicationBase > _application_instance;

template< typename Derived >
class Application : private ApplicationBase
{
public:
	static volatile int _initializer_eval;
};

template< typename Derived >
volatile int Application< Derived >::_initializer_eval = [] { _application_initializer = [] { return std::static_pointer_cast< void >( std::make_shared< Derived >() ); }; return 1; }();

ORB_NAMESPACE_END
