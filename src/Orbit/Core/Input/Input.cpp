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
		Point offset_from_center;
		Point movement_since_last_frame;
		bool  held     : 1;
		bool  pressed  : 1;
		bool  released : 1;
	};

	struct FPSCursor
	{
		bool enabled = false;
	};

	static std::map< Key, KeyState >   key_states;
	static std::map< size_t, Pointer > pointers;
	static FPSCursor                   fps_cursor;
	static Point                       center;

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

	void SetPointerPressed( size_t index, Point pos )
	{
		Pointer& pointer = pointers[ index ];

		pointer.offset_from_center        = pos - center;
		pointer.movement_since_last_frame = Point();
		pointer.held                      = true;
		pointer.pressed                   = true;
	}

	void SetPointerReleased( size_t index, Point pos )
	{
		Pointer& pointer = pointers[ index ];

		pointer.offset_from_center        = pos - center;
		pointer.movement_since_last_frame = Point();
		pointer.held                      = false;
		pointer.released                  = true;
	}

	void SetPointerPos( size_t index, Point pos )
	{
		pointers[ index ].movement_since_last_frame = pos - center;
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

	Point GetPointerPos( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return center + it->second.offset_from_center;
		}

		return Point();
	}

	Point GetPointerMove( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return it->second.movement_since_last_frame;
		}

		return Point();
	}

	void SetFPSCursor( bool enable )
	{
		fps_cursor.enabled = enable;

	#if defined( ORB_OS_WINDOWS )
		
		/* Toggle cursor visibility */
		ShowCursor( !enable );

	#else

		if( Window* window = Window::GetPtr(); window != nullptr )
		{
			auto& details = window->GetPrivateDetails();
			
			static Cursor invisible_cursor = [ & ]( void ) -> Cursor
			{
				char   data[ 1 ] { 0x00 };
				Pixmap pixmap;
				
				if( ( pixmap = XCreateBitmapFromData( details.display, details.window, data, 1, 1 ) ) == None )
				{
					return 0;
				}
				
				XColor color;
				Cursor cursor = XCreatePixmapCursor( details.display, pixmap, pixmap, &color, &color, 0, 0 );
				
				XFreePixmap( details.display, pixmap );
				
				return cursor;
			}();
			
			XDefineCursor( details.display, details.window, enable ? invisible_cursor : None );
		}

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
				center.x = ( client_rect.left + client_rect.right  ) / 2;
				center.y = ( client_rect.top  + client_rect.bottom ) / 2;

				if( fps_cursor.enabled )
				{
					POINT cursor_pos { center.x, center.y };

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
			it.second.offset_from_center       += it.second.movement_since_last_frame;
			it.second.movement_since_last_frame = Point();
			it.second.pressed                   = false;
			it.second.released                  = false;
		}
	}
}

ORB_NAMESPACE_END
