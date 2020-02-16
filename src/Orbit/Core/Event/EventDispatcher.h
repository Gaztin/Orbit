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
#include "Orbit/Core/Event/EventSubscription.h"
#include "Orbit/Core/Utility/Utility.h"

#include <algorithm>
#include <functional>
#include <list>
#include <mutex>
#include <tuple>
#include <vector>

ORB_NAMESPACE_BEGIN

template< typename... Types >
class EventDispatcher
{
public:

	         EventDispatcher( void ) = default;
	virtual ~EventDispatcher( void ) = default;

	ORB_DISABLE_COPY_AND_MOVE( EventDispatcher );

public:

	template< typename Functor >
	[[ nodiscard ]] EventSubscription Subscribe( Functor&& functor ) const
	{
		using Arg = std::remove_const_t< std::remove_reference_t< FirstArgument< Functor > > >;

		const uint64_t unique_id = GenerateUniqueID();

		{
			const Queue< Arg >& queue      = std::get< Queue< Arg > >( queues_ );
			Subscriber< Arg >   subscriber = { unique_id, std::forward< Functor >( functor ) };
			std::scoped_lock    lock       = std::scoped_lock( queue.mutex );
			queue.subscribers.push_back( subscriber );
		}

		EventSubscription::Deleter deleter;
		deleter.user_data = this;
		deleter.functor   = []( uint64_t id, const void* user_data )
		{
			const EventDispatcher* self = reinterpret_cast< const EventDispatcher* >( user_data );
			self->Unsubscribe< Arg >( id );
		};

		return EventSubscription( unique_id, deleter );
	}

	template< typename T >
	void QueueEvent( const T& e )
	{
		Queue< T >&      queue = std::get< Queue< T > >( queues_ );
		std::scoped_lock lock  = std::scoped_lock( queue.mutex );

		queue.events.push_back( e );
	}

protected:

	void SendEvents()
	{
		( SendEventsInQueue( std::get< Queue< Types > >( queues_ ) ), ... );
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
		std::vector< T > events;
		mutable std::list< Subscriber< T > > subscribers;
		mutable std::mutex mutex;
	};

private:

	uint64_t GenerateUniqueID() const
	{
		static std::atomic_uint64_t counter = 0;
		return ++counter;
	}

	template< typename T >
	void Unsubscribe( uint64_t id ) const
	{
		const Queue< T >& queue = std::get< Queue< T > >( queues_ );
		std::scoped_lock  lock  = std::scoped_lock( queue.mutex );

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

private:

	std::tuple< Queue< Types >... > queues_;

};

ORB_NAMESPACE_END
