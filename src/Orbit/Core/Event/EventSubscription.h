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
#include <atomic>
#include <functional>

#include "Orbit/Core/Core.h"

ORB_NAMESPACE_BEGIN

class ORB_API_CORE EventSubscription
{
public:
	struct Deleter
	{
		void* dispatcher_ptr;
		void( *functor )( void* caller, uint64_t id );
	};

	EventSubscription();
	EventSubscription( uint64_t id, Deleter deleter );
	EventSubscription( const EventSubscription& other );
	EventSubscription( EventSubscription&& other );
	~EventSubscription();

	EventSubscription& operator=( const EventSubscription& other );
	EventSubscription& operator=( EventSubscription&& other );

private:
	struct ControlBlock
	{
		std::atomic_uint64_t m_ref_count;
	};

	uint64_t      m_id;
	Deleter       m_deleter;
	ControlBlock* m_control_block;
};

ORB_NAMESPACE_END
