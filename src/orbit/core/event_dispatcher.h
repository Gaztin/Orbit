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
#include <algorithm>
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
	struct subscription
	{
	public:
		~subscription() { unsubscriber(id); }

		uint64_t id;
		std::function<void(uint64_t)> unsubscriber;
	};

	event_dispatcher() = default;
	virtual ~event_dispatcher() = default;

	template<typename Functor>
	[[nodiscard]] std::shared_ptr<subscription> subscribe(Functor&& functor)
	{
		static uint64_t uniqueId = 0;
		std::shared_ptr<subscription> sub = std::make_shared<subscription>();
		sub->id = ++uniqueId;
		sub->unsubscriber = [this](uint64_t id) { unsubscribe(id); };
		m_subscribers.push_back(subscriber{sub->id, std::forward<Functor>(functor)});
		return sub;
	}

	void unsubscribe(uint64_t subscriptionId)
	{
		auto it = std::find_if(m_subscribers.begin(), m_subscribers.end(),
			[subscriptionId](const subscriber& sub) { return sub.subscriptionId == subscriptionId; });
		if (it == m_subscribers.end())
			return;

		m_subscribers.erase(it);
	}

	void queue_event(const EventType& evt)
	{
		m_mutex.lock();
		m_eventQueue.push_back(evt);
		m_mutex.unlock();
	}

	void send_events()
	{
		for (const EventType& evt : m_eventQueue)
			for (subscriber& sub : m_subscribers)
				sub.functor(evt);

		m_eventQueue.clear();
	}

private:
	struct subscriber
	{
		uint64_t subscriptionId;
		std::function<void(const EventType&)> functor;
	};

	std::vector<subscriber> m_subscribers;
	std::vector<EventType> m_eventQueue;
	std::mutex m_mutex;
};

}
