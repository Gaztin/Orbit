/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "orbit/core.h"

namespace orb
{
	template< typename EventType >
	class event_dispatcher
	{
	public:
		using event_t          = EventType;
		using subscription_ptr = std::shared_ptr< uint64_t >;

		event_dispatcher() = default;
		virtual ~event_dispatcher() = default;

		template< typename Functor >
		[ [ nodiscard ] ] subscription_ptr subscribe( Functor&& functor )
		{
			static uint64_t uniqueId = 0;
			m_subscribers.push_back( subscriber{ ++uniqueId, std::forward< Functor >( functor ) } );
			subscription_ptr sub( &m_subscribers.back().id, [ this ]( uint64_t* id ) { unsubscribe( *id ); } );
			return sub;
		}

		void queue_event( const EventType& et )
		{
			m_mutex.lock();
			m_eventQueue.push_back( et );
			m_mutex.unlock();
		}

		void send_events()
		{
			for( const EventType& et : m_eventQueue )
				for( subscriber& sub : m_subscribers )
					sub.functor( et );

			m_eventQueue.clear();
		}

	private:
		struct subscriber
		{
			uint64_t id;
			std::function< void( const EventType& ) > functor;
		};

		void unsubscribe( uint64_t id )
		{
			auto it = std::find_if( m_subscribers.begin(), m_subscribers.end(), [ id ]( const subscriber& sub ) { return sub.id == id; } );
			if( it == m_subscribers.end() )
				return;

			m_subscribers.erase( it );
		}

		std::list< subscriber >  m_subscribers;
		std::vector< EventType > m_eventQueue;
		std::mutex               m_mutex;
	};
}
