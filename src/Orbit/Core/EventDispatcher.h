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
#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "Orbit/Core.h"

ORB_NAMESPACE_BEGIN

template< typename T >
class EventDispatcher
{
public:
	using Event           = T;
	using SubscriptionPtr = std::shared_ptr< uint64_t >;

	EventDispatcher() = default;
	virtual ~EventDispatcher() = default;

	template< typename Functor >
	[[ nodiscard ]] SubscriptionPtr subscribe( Functor&& functor )
	{
		static uint64_t unique_id = 0;
		m_subscribers.push_back( Subscriber{ ++unique_id, std::forward< Functor >( functor ) } );
		SubscriptionPtr sub( &m_subscribers.back().id, [ this ]( uint64_t* id ) { Unsubscribe( *id ); } );
		return sub;
	}

	void QueueEvent( const Event& e )
	{
		m_mutex.lock();
		m_event_queue.push_back( e );
		m_mutex.unlock();
	}

	void SendEvents()
	{
		for( const Event& e : m_event_queue )
		{
			for( Subscriber& sub : m_subscribers )
				sub.functor( e );
		}

		m_event_queue.clear();
	}

private:
	struct Subscriber
	{
		uint64_t id;
		std::function< void( const Event& ) > functor;
	};

	void Unsubscribe( uint64_t id )
	{
		auto it = std::find_if( m_subscribers.begin(), m_subscribers.end(), [ id ]( const Subscriber& sub ) { return sub.id == id; } );
		if( it == m_subscribers.end() )
			return;

		m_subscribers.erase( it );
	}

	std::list< Subscriber > m_subscribers;
	std::vector< Event >    m_event_queue;
	std::mutex              m_mutex;
};

ORB_NAMESPACE_END
