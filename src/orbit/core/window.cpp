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

#include "window.h"

namespace orb
{
	window::window( uint32_t width, uint32_t height )
		: m_handle( platform::create_window_handle( width, height ) )
		, m_open( true )
	{
		platform::set_window_user_data( m_handle, *this );
	}

	void window::poll_events()
	{
		std::optional< platform::message > msg;
		while( ( msg = platform::peek_message( m_handle ) ) )
		{
			platform::process_message( *this, *msg );
		}

		send_events();
	}

	void window::set_title( const std::string& title )
	{
		platform::set_window_title( m_handle, title );
	}

	void window::set_pos( uint32_t x, uint32_t y )
	{
		platform::set_window_position( m_handle, x, y );
	}

	void window::set_size( uint32_t width, uint32_t height )
	{
		platform::set_window_size( m_handle, width, height );
	}

	void window::show()
	{
		platform::set_window_visibility( m_handle, true );
	}

	void window::hide()
	{
		platform::set_window_visibility( m_handle, false );
	}
}
