/*
* Copyright (c) 2018 Sebastian Kylander http://gaztin.com/
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
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

#include "orbit/core.h"

namespace orb
{

template<typename EventType>
class event_dispatcher
{
public:
	struct subscription_t
	{
		uint64_t id;
		event_dispatcher<EventType>* source;
	};

	struct subscriber_t
	{
		subscription_t subscription;
		std::function<void(const EventType&)> functor;
	};

	event_dispatcher() = default;

	template<typename Functor>
	subscription_t subscribe(Functor&& functor)
	{
		static uint64_t uniqueId = 0;
		m_subscribers.push_back({{++uniqueId, this}, std::forward<Functor>(functor)});
		return m_subscribers.back().subscription;
	}

	static void unsubscribe(subscription_t& subscription)
	{
		for (auto it = subscription.source->m_subscribers.begin(); it != subscription.source->m_subscribers.end(); ++it)
		{
			if (it->subscription.id == subscription.id)
			{
				subscription.source->m_subscribers.erase(it);
				subscription.id = 0;
				subscription.source = nullptr;
				return;
			}
		}
	}

	void queue_event(const EventType& e)
	{
		m_mutex.lock();
		m_eventQueue.push_back(e);
		m_mutex.unlock();
	}

protected:
	void send_events()
	{
		for (const EventType& e : m_eventQueue)
		{
			for (subscriber_t& subscriber : m_subscribers)
			{
				subscriber.functor(e);
			}
		}
		m_eventQueue.clear();
	}

private:
	std::vector<subscriber_t> m_subscribers;
	std::vector<EventType> m_eventQueue;
	std::mutex m_mutex;
};

}
