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
		Pos  offset_from_center;
		Pos  movement_since_last_frame;
		bool held     : 1;
		bool pressed  : 1;
		bool released : 1;
	};

	struct FPSCursor
	{
		bool enabled = false;
	};

	static std::map< Key, KeyState >   key_states;
	static std::map< size_t, Pointer > pointers;
	static FPSCursor                   fps_cursor;
	static Pos                         center;

	static Pos CalcOffset( Pos from, Pos to )
	{
		const auto& [ fx, fy ] = from;
		const auto& [ tx, ty ] = to;

		return std::make_pair( tx - fx, ty - fy );
	}

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

		pointer.offset_from_center        = CalcOffset( center, pos );
		pointer.movement_since_last_frame = std::make_pair( 0, 0 );
		pointer.held                      = true;
		pointer.pressed                   = true;
	}

	void SetPointerReleased( size_t index, Pos pos )
	{
		Pointer& pointer = pointers[ index ];

		pointer.offset_from_center        = CalcOffset( center, pos );
		pointer.movement_since_last_frame = std::make_pair( 0, 0 );
		pointer.held                      = false;
		pointer.released                  = true;
	}

	void SetPointerPos( size_t index, Pos pos )
	{
		pointers[ index ].movement_since_last_frame = CalcOffset( center, pos );
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
			return it->second.offset_from_center;
		}

		return { };
	}

	Pos GetPointerMove( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			auto [ mx, my ] = it->second.movement_since_last_frame;

			return std::make_tuple( mx, my );
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

	}

	void ResetStates( void )
	{

	#if defined( ORB_OS_WINDOWS )

		if( Window* window = Window::GetPtr(); window != nullptr )
		{
			HWND hwnd = window->GetPrivateDetails().hwnd;
			RECT client_rect;

			if( GetClientRect( hwnd, &client_rect ) )
			{
				auto& [ cx, cy ] = center;
				cx = ( client_rect.left + client_rect.right  ) / 2;
				cy = ( client_rect.top  + client_rect.bottom ) / 2;

				if( fps_cursor.enabled )
				{
					POINT cursor_pos;
					cursor_pos.x = cx;
					cursor_pos.y = cy;

					ClientToScreen( hwnd, &cursor_pos );
					SetCursorPos( cursor_pos.x, cursor_pos.y );
				}
			}
		}

	#else

	#error Set center var and cursor pos

	#endif

		for( auto& it : key_states )
		{
			it.second.pressed  = false;
			it.second.released = false;
		}

		for( auto& it : pointers )
		{
			auto& [ ox, oy ] = it.second.offset_from_center;
			auto& [ mx, my ] = it.second.movement_since_last_frame;

			ox += mx;
			oy += my;
			mx  = 0;
			my  = 0;

			it.second.pressed      = false;
			it.second.released     = false;
		}
	}
}

ORB_NAMESPACE_END
