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

#include "EventSubscription.h"

ORB_NAMESPACE_BEGIN

EventSubscription::EventSubscription( void )
	: m_id            { 0 }
	, m_deleter       { nullptr, nullptr }
	, m_control_block { nullptr }
{
}

EventSubscription::EventSubscription( uint64_t id, Deleter deleter )
	: m_id            { id }
	, m_deleter       { deleter }
	, m_control_block { new ControlBlock{ 1 } }
{
}

EventSubscription::EventSubscription( const EventSubscription& other )
	: m_id            { other.m_id }
	, m_deleter       { other.m_deleter }
	, m_control_block { other.m_control_block }
{
	if( m_control_block )
	{
		++m_control_block->m_ref_count;
	}
}

EventSubscription::EventSubscription( EventSubscription&& other )
	: m_id            { other.m_id }
	, m_deleter       { other.m_deleter }
	, m_control_block { other.m_control_block }
{
	other.m_id                = 0;
	other.m_deleter.user_data = nullptr;
	other.m_deleter.functor   = nullptr;
	other.m_control_block     = nullptr;
}

EventSubscription::~EventSubscription( void )
{
	if( m_control_block )
	{
		if( --m_control_block->m_ref_count == 0 )
		{
			m_deleter.functor( m_id, m_deleter.user_data );

			delete m_control_block;
		}
	}
}

EventSubscription& EventSubscription::operator=( const EventSubscription& other )
{
	m_id            = other.m_id;
	m_deleter       = other.m_deleter;
	m_control_block = other.m_control_block;

	if( m_control_block )
	{
		++m_control_block->m_ref_count;
	}

	return *this;
}

EventSubscription& EventSubscription::operator=( EventSubscription&& other )
{
	m_id            = other.m_id;
	m_deleter       = other.m_deleter;
	m_control_block = other.m_control_block;

	other.m_id                = 0;
	other.m_deleter.user_data = nullptr;
	other.m_deleter.functor   = nullptr;
	other.m_control_block     = nullptr;

	return *this;
}

ORB_NAMESPACE_END
