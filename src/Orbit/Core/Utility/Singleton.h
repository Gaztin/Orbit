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
#include <cassert>

#include "Orbit/Core/Core.h"

ORB_NAMESPACE_BEGIN

template< typename Derived, bool AutomaticInitialization >
class Singleton;

template< typename Derived >
class Singleton< Derived, false >
{
public:

	static Derived& GetInstance( void )
	{
		assert( s_instance != nullptr );
		return *s_instance;
	}

	static Derived* GetInstancePtr( void )
	{
		assert( s_instance != nullptr );
		return s_instance;
	}

protected:

	Singleton( void )
	{
		assert( s_instance == nullptr );
		s_instance = static_cast< Derived* >( this );
	}

	~Singleton( void )
	{
		assert( s_instance == this );
		s_instance = nullptr;
	}

private:

	static Derived* s_instance;

};

template< typename Derived >
class Singleton< Derived, true >
{
public:

	static Derived& GetInstance( void )
	{
		static Derived instance { };
		return instance;
	}

};

template< typename Derived >
Derived* Singleton< Derived, false >::s_instance = nullptr;

ORB_NAMESPACE_END
