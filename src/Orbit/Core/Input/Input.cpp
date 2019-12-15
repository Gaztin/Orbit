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

#include "Input.h"

#include "Orbit/Core/Widget/Window.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#endif

ORB_NAMESPACE_BEGIN

namespace Input
{
	struct KeyState
	{
		bool held     : 1;
		bool pressed  : 1;
		bool released : 1;
	};

	struct Pointer
	{
		Pos  current_pos;
		Pos  previous_pos;
		bool held     : 1;
		bool pressed  : 1;
		bool released : 1;
	};

	struct FPSCursor
	{
		Pos  pos;
		Pos  offset_from_origin;
		bool enabled = false;
	};

	static std::map< Key, KeyState >   key_states;
	static std::map< size_t, Pointer > pointers;
	static FPSCursor                   fps_cursor;

	void SetKeyPressed( Key key )
	{
		KeyState& state = key_states[ key ];

		state.held     = true;
		state.pressed  = true;
	}

	void SetKeyReleased( Key key )
	{
		KeyState& state = key_states[ key ];

		state.held     = false;
		state.released = true;
	}

	bool GetKeyPressed( Key key )
	{
		if( auto it = key_states.find( key ); it != key_states.end() )
		{
			return it->second.pressed;
		}

		return false;
	}

	bool GetKeyReleased( Key key )
	{
		if( auto it = key_states.find( key ); it != key_states.end() )
		{
			return it->second.released;
		}

		return false;
	}

	bool GetKeyHeld( Key key )
	{
		if( auto it = key_states.find( key ); it != key_states.end() )
		{
			return it->second.held;
		}

		return false;
	}

	void SetPointerPressed( size_t index, Pos pos )
	{
		Pointer& pointer = pointers[ index ];

		pointer.current_pos  = pos;
		pointer.previous_pos = pos;
		pointer.held         = true;
		pointer.pressed      = true;
	}

	void SetPointerReleased( size_t index, Pos pos )
	{
		Pointer& pointer = pointers[ index ];

		pointer.current_pos  = pos;
		pointer.previous_pos = pos;
		pointer.held         = false;
		pointer.released     = true;
	}

	void SetPointerPos( size_t index, Pos pos )
	{
		if( fps_cursor.enabled )
		{
			std::get< 0 >( pos ) += std::get< 0 >( fps_cursor.offset_from_origin );
			std::get< 1 >( pos ) += std::get< 1 >( fps_cursor.offset_from_origin );

			/* FPS cursors only work on desktop systems where all pointers are going to have the same position in the
			 * end, so setting the position regardless of the pointer index like this should be fine */
			fps_cursor.pos = pos;
		}

		pointers[ index ].current_pos = pos;
	}

	bool GetPointerPressed( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return it->second.pressed;
		}

		return false;
	}

	bool GetPointerReleased( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return it->second.released;
		}

		return false;
	}

	bool GetPointerHeld( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return it->second.held;
		}

		return false;
	}

	Pos GetPointerPos( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return it->second.current_pos;
		}

		return { };
	}

	Pos GetPointerMove( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			const auto& [ curr_x, curr_y ] = it->second.current_pos;
			const auto& [ prev_x, prev_y ] = it->second.previous_pos;

			return std::make_tuple( curr_x - prev_x, curr_y - prev_y );
		}

		return { };
	}

	void SetFPSCursor( bool enable )
	{
		fps_cursor.enabled = enable;

		/* Toggle cursor visibility */
	#if defined( ORB_OS_WINDOWS )
		ShowCursor( !enable );
	#else
	#  error Implement cursor visibility
	#endif

	#if defined( ORB_OS_WINDOWS )

		Window* window = Window::GetPtr();

		if( window != nullptr )
		{
			HWND  window_handle = window->GetPrivateDetails().hwnd;
			POINT cursor_pos;

			if( GetCursorPos( &cursor_pos ) && ScreenToClient( window_handle, &cursor_pos ) )
			{
				auto&[ fps_x, fps_y ] = fps_cursor.pos;

				fps_x = cursor_pos.x;
				fps_y = cursor_pos.y;
			}
		}

	#else

	#  error Grab cursor pos

	#endif

	}

	void ResetStates( void )
	{
		if( fps_cursor.enabled )
		{

		#if defined( ORB_OS_WINDOWS )

			if( Window* window = Window::GetPtr(); window != nullptr )
			{
				HWND hwnd = window->GetPrivateDetails().hwnd;
				RECT window_rect;

				if( GetWindowRect( hwnd, &window_rect ) && SetCursorPos( ( window_rect.left + window_rect.right  ) / 2, ( window_rect.top  + window_rect.bottom ) / 2 ) )
				{
					auto [ cur_x, cur_y ] = fps_cursor.pos;
					RECT client_rect;

					if( GetClientRect( hwnd, &client_rect ) )
					{
						const int caption_height = GetSystemMetrics( SM_CYCAPTION );
						const int border_height  = GetSystemMetrics( SM_CYBORDER );

						/* I am a little confused as to why we need to subtract the caption height (excluding border thickness)
						 * from the Y position, but it seems to work the way we want it to for now */
						fps_cursor.offset_from_origin = std::make_tuple( -( ( ( client_rect.left + client_rect.right                                       ) / 2 ) - cur_x ),
						                                                 -( ( ( client_rect.top  + client_rect.bottom - ( caption_height - border_height ) ) / 2 ) - cur_y ) );
					}
				}
			}

		#endif

		}

		for( auto& it : key_states )
		{
			it.second.pressed  = false;
			it.second.released = false;
		}

		for( auto& it : pointers )
		{
			it.second.previous_pos = it.second.current_pos;
			it.second.pressed      = false;
			it.second.released     = false;
		}
	}
}

ORB_NAMESPACE_END
