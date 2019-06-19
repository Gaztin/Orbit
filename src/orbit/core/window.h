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

#include <string_view>

#include "orbit/core/event_dispatcher.h"
#include "orbit/core/events/window_event.h"
#include "orbit/core/internal/window_impl.h"

namespace orb
{
	class ORB_API_CORE window : public event_dispatcher< window_event >
	{
	public:
		window( uint32_t width, uint32_t height, window_impl_type implType = DefaultWindowImpl );

		void poll_events();
		void set_title( std::string_view title );
		void set_pos( uint32_t x, uint32_t y );
		void set_size( uint32_t width, uint32_t height );
		void show();
		void hide();
		void close() { m_open = false; }

		operator bool() const { return m_open; }

		window_impl_type     get_impl_type ( void ) const;
		window_impl_storage* get_impl_ptr  ( void ) { return &m_storage; }

	private:
		window_impl_storage m_storage;
		bool                m_open;

	#if ( __ORB_NUM_WINDOW_IMPLS > 1 )
		const window_impl_type m_implType;
	#endif

	};
}
