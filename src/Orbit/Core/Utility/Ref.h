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

#include <memory>

ORB_NAMESPACE_BEGIN

template< typename T >
class Ref
{
public:

	Ref( void )
		: ptr_( nullptr )
	{
	}

	Ref( const Ref& other )
		: ptr_( other.ptr_ )
	{
	}

	Ref( Ref&& other )
		: ptr_( other.ptr_ )
	{
		other.ptr_ = nullptr;
	}

	Ref( T& ref )
		: ptr_( std::addressof( ref ) )
	{
	}

public:

	Ref& operator=( const Ref& other )
	{
		ptr_ = other.ptr_;
		return *this;
	}

	Ref& operator=( Ref&& other )
	{
		ptr_       = other.ptr_;
		other.ptr_ = nullptr;

		return *this;
	}

	Ref& operator=( T& ref )
	{
		ptr_ = std::addressof( ref );
		return *this;
	}

public:

	operator bool      ( void ) const { return ( ptr_ != nullptr ); }
	T*       operator->( void )       { return ptr_; }
	const T* operator->( void ) const { return ptr_; }

private:

	T* ptr_;

};

ORB_NAMESPACE_END
