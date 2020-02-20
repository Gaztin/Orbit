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

#pragma once
#include "Orbit/Core/Core.h"

#include <cassert>

ORB_NAMESPACE_BEGIN

template< typename Derived >
class ManualSingleton
{
public:

	static Derived& GetInstance( void )
	{
		assert( instance_ != nullptr );
		return *instance_;
	}

	static Derived* GetInstancePtr( void )
	{
		return instance_;
	}

protected:

	ManualSingleton( void )
	{
		assert( instance_ == nullptr );
		instance_ = static_cast< Derived* >( this );
	}

	~ManualSingleton( void )
	{
		assert( instance_ == this );
		instance_ = nullptr;
	}

private:

	static Derived* instance_;

};

template< typename Derived >
class Singleton
{
public:

	static Derived& GetInstance( void )
	{
		static Derived instance{ };
		return instance;
	}

};

template< typename Derived >
Derived* ManualSingleton< Derived >::instance_ = nullptr;

ORB_NAMESPACE_END
