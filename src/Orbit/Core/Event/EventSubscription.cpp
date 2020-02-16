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

#include "EventSubscription.h"

ORB_NAMESPACE_BEGIN

EventSubscription::EventSubscription( void )
	: id_           { 0 }
	, deleter_      { nullptr, nullptr }
	, control_block_{ nullptr }
{
}

EventSubscription::EventSubscription( uint64_t id, Deleter deleter )
	: id_           { id }
	, deleter_      { deleter }
	, control_block_{ new ControlBlock{ 1 } }
{
}

EventSubscription::EventSubscription( const EventSubscription& other )
	: id_           { other.id_ }
	, deleter_      { other.deleter_ }
	, control_block_{ other.control_block_ }
{
	if( control_block_ )
		++control_block_->ref_count;
}

EventSubscription::EventSubscription( EventSubscription&& other )
	: id_           { other.id_ }
	, deleter_      { other.deleter_ }
	, control_block_{ other.control_block_ }
{
	other.id_                = 0;
	other.deleter_.user_data = nullptr;
	other.deleter_.functor   = nullptr;
	other.control_block_     = nullptr;
}

EventSubscription::~EventSubscription( void )
{
	if( control_block_ )
	{
		if( ( --control_block_->ref_count ) == 0 )
		{
			deleter_.functor( id_, deleter_.user_data );

			delete control_block_;
		}
	}
}

EventSubscription& EventSubscription::operator=( const EventSubscription& other )
{
	id_            = other.id_;
	deleter_       = other.deleter_;
	control_block_ = other.control_block_;

	if( control_block_ )
		++control_block_->ref_count;

	return *this;
}

EventSubscription& EventSubscription::operator=( EventSubscription&& other )
{
	id_                      = other.id_;
	deleter_                 = other.deleter_;
	control_block_           = other.control_block_;

	other.id_                = 0;
	other.deleter_.user_data = nullptr;
	other.deleter_.functor   = nullptr;
	other.control_block_     = nullptr;

	return *this;
}

ORB_NAMESPACE_END
