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
#include <tuple>
#include <vector>

#include "Orbit/Core/Event/EventSubscription.h"

ORB_NAMESPACE_BEGIN

template< typename... Types >
class EventDispatcher
{
public:

	template< typename T >
	struct Subscriber
	{
		uint64_t id;
		std::function< void( const T& ) > function;
	};

	template< typename T >
	struct Queue
	{
		std::vector< T > events;
		std::list< Subscriber< T > > subscribers;
	};

	EventDispatcher() = default;
	virtual ~EventDispatcher() = default;

	template< typename T, typename Functor >
	[[ nodiscard ]] EventSubscription Subscribe( Functor&& functor )
	{
		Queue< T >& queue = std::get< Queue< T > >( m_queues );
		Subscriber< T > sub { GenerateUniqueID(), std::forward< Functor >( functor ) };
		queue.subscribers.push_back( sub );

		EventSubscription::Deleter deleter;
		deleter.user_data = this;
		deleter.functor   = []( uint64_t id, void* user_data )
		{
			EventDispatcher* self = reinterpret_cast< EventDispatcher* >( user_data );
			self->Unsubscribe< T >( id );
		};

		return EventSubscription( sub.id, deleter );
	}

	template< typename T >
	void QueueEvent( const T& e )
	{
		Queue< T >& queue = std::get< Queue< T > >( m_queues );
		queue.events.push_back( e );
	}

	void SendEvents()
	{
		( UpdateQueue( std::get< Queue< Types > >( m_queues ) ), ... );
	}

private:

	uint64_t GenerateUniqueID()
	{
		static std::atomic_uint64_t counter = 0;
		return ++counter;
	}

	template< typename T >
	void Unsubscribe( uint64_t id )
	{
		Queue< T >& queue = std::get< Queue< T > >( m_queues );

		for( auto it = queue.subscribers.begin(); it != queue.subscribers.end(); ++it )
		{
			if( it->id == id )
			{
				queue.subscribers.erase( it );
				break;
			}
		}
	}

	template< typename T >
	void UpdateQueue( Queue< T >& queue )
	{
		for( const T& e : queue.events )
		{
			for( Subscriber< T >& sub : queue.subscribers )
				sub.function( e );
		}

		queue.events.clear();
	}

	std::tuple< Queue< Types >... > m_queues;

};

ORB_NAMESPACE_END
