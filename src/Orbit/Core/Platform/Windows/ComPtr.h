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

#if defined( ORB_OS_WINDOWS )
#  include <memory>
#  include <type_traits>

#  include <Windows.h>

ORB_NAMESPACE_BEGIN

template< typename T
	, typename = typename std::enable_if_t< std::is_base_of_v< IUnknown, T > > >
class ComPtr final
{
public:

	ComPtr( void )
		: ptr_( nullptr )
	{
	}

	ComPtr( const ComPtr& other )
		: ptr_( other.ptr_ )
	{
		if( ptr_ )
			ptr_->AddRef();
	}

	ComPtr( ComPtr&& other )
		: ptr_( other.ptr_ )
	{
		other.ptr_ = nullptr;
	}

	explicit ComPtr( T* ptr )
		: ptr_( ptr )
	{
	}

	~ComPtr( void )
	{
		if( ptr_ )
			ptr_->Release();
	}

public:

	ComPtr& operator=( const ComPtr& other )
	{
		ptr_ = other.ptr_;

		if( ptr_ )
			ptr_->AddRef();

		return *this;
	}

	ComPtr& operator=( ComPtr&& other )
	{
		ptr_       = other.ptr_;
		other.ptr_ = nullptr;

		return *this;
	}

	ComPtr& operator=( std::nullptr_t )
	{
		if( ptr_ )
			ptr_->Release();

		ptr_ = nullptr;

		return *this;
	}

public:

	operator bool      ( void )           const { return ( ptr_ != nullptr ); }
	bool     operator==( const T* other ) const { return ( ptr_ == other ); }
	T*       operator->( void )                 { return ptr_; }
	const T* operator->( void )           const { return ptr_; }

public:

	T* ptr_;

};

ORB_NAMESPACE_END

#endif // ORB_OS_WINDOWS
