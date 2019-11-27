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
#include <mutex>
#include <tuple>
#include <vector>

#include "Orbit/Core/Event/EventSubscription.h"
#include "Orbit/Core/Utility/Utility.h"

ORB_NAMESPACE_BEGIN

template< typename... Types >
class EventDispatcher
{
public:

	EventDispatcher() = default;
	virtual ~EventDispatcher() = default;

	ORB_DISABLE_COPY_AND_MOVE( EventDispatcher );

	template< typename Functor >
	[[ nodiscard ]] EventSubscription Subscribe( Functor&& functor )
	{
		using Arg = std::remove_const_t< std::remove_reference_t< FirstArgument< Functor > > >;

		const uint64_t unique_id = GenerateUniqueID();

		{
			Queue< Arg >&     queue      = std::get< Queue< Arg > >( m_queues );
			Subscriber< Arg > subscriber = { unique_id, std::forward< Functor >( functor ) };
			std::scoped_lock  lock       = std::scoped_lock( queue.mutex );
			queue.subscribers.push_back( subscriber );
		}

		EventSubscription::Deleter deleter;
		deleter.user_data = this;
		deleter.functor   = []( uint64_t id, void* user_data )
		{
			EventDispatcher* self = reinterpret_cast< EventDispatcher* >( user_data );
			self->Unsubscribe< Arg >( id );
		};

		return EventSubscription( unique_id, deleter );
	}

protected:

	template< typename T >
	void QueueEvent( const T& e )
	{
		Queue< T >&      queue = std::get< Queue< T > >( m_queues );
		std::scoped_lock lock  = std::scoped_lock( queue.mutex );

		queue.events.push_back( e );
	}

	void SendEvents()
	{
		( SendEventsInQueue( std::get< Queue< Types > >( m_queues ) ), ... );
	}

private:

	template< typename T >
	struct Subscriber
	{
		uint64_t id;
		std::function< void( const T& ) > function;
	};

	template< typename T >
	struct Queue
	{
		std::mutex mutex;
		std::vector< T > events;
		std::list< Subscriber< T > > subscribers;
	};

	uint64_t GenerateUniqueID() const
	{
		static std::atomic_uint64_t counter = 0;
		return ++counter;
	}

	template< typename T >
	void Unsubscribe( uint64_t id )
	{
		Queue< T >&      queue = std::get< Queue< T > >( m_queues );
		std::scoped_lock lock  = std::scoped_lock( queue.mutex );

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
	void SendEventsInQueue( Queue< T >& queue ) const
	{
		std::scoped_lock lock( queue.mutex );

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
