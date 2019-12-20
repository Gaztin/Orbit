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
#elif defined( ORB_OS_MACOS )
#  include <AppKit/AppKit.h>
#  include <CoreGraphics/CoreGraphics.h>
#endif

ORB_NAMESPACE_BEGIN

namespace Input
{
	struct BinaryState
	{
		bool held     : 1;
		bool pressed  : 1;
		bool released : 1;
	};

	using ButtonState = BinaryState;
	using KeyState    = BinaryState;

	struct Pointer
	{
		Point current_pos;
		Point previous_pos;
	};

	struct FPSCursor
	{
		bool enabled = false;
	};

	static std::map< Key, KeyState >       key_states;
	static std::map< Button, ButtonState > button_states;
	static std::map< size_t, Pointer >     pointers;
	static FPSCursor                       fps_cursor;
	static Point                           center;

	void SetKeyPressed( Key key )
	{
		ButtonState& state = key_states[ key ];

		state.held     = true;
		state.pressed  = true;
	}

	void SetKeyReleased( Key key )
	{
		ButtonState& state = key_states[ key ];

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

	void SetButtonPressed( Button button )
	{
		ButtonState& state = button_states[ button ];

		state.held    = true;
		state.pressed = true;
	}

	void SetButtonReleased( Button button )
	{
		ButtonState& state = button_states[ button ];

		state.held     = false;
		state.released = true;
	}

	bool GetButtonPressed( Button button )
	{
		if( auto it = button_states.find( button ); it != button_states.end() )
		{
			return it->second.pressed;
		}

		return false;
	}

	bool GetButtonReleased( Button button )
	{
		if( auto it = button_states.find( button ); it != button_states.end() )
		{
			return it->second.released;
		}

		return false;
	}

	bool GetButtonHeld( Button button )
	{
		if( auto it = button_states.find( button ); it != button_states.end() )
		{
			return it->second.held;
		}

		return false;
	}

	void SetPointerPos( size_t index, Point pos )
	{
		Pointer& pointer = pointers[ index ];

		pointer.current_pos = pos;

		if( fps_cursor.enabled )
		{
			pointer.previous_pos = center;
		}
	}

	Point GetPointerPos( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return it->second.current_pos;
		}

		return Point();
	}

	Point GetPointerMove( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return ( it->second.current_pos - it->second.previous_pos );
		}

		return Point();
	}

	void SetFPSCursor( bool enable )
	{
		fps_cursor.enabled = enable;

	#if defined( ORB_OS_WINDOWS )
		
		/* Toggle cursor visibility */
		ShowCursor( !enable );

	#elif defined( ORB_OS_LINUX )

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

	#elif defined( ORB_OS_MACOS )

		if( enable )
		{
			[ NSCursor hide ];
		}
		else
		{
			[ NSCursor unhide ];
		}

	#endif

	}

	void ResetStates( void )
	{

	#if defined( ORB_OS_WINDOWS )

		if( fps_cursor.enabled )
		{
			if( Window* window = Window::GetPtr(); window != nullptr )
			{
				HWND hwnd = window->GetPrivateDetails().hwnd;
				RECT client_rect;

				if( GetClientRect( hwnd, &client_rect ) )
				{
					POINT cursor_pos;

					center.x     = ( client_rect.left + client_rect.right ) / 2;
					center.y     = ( client_rect.top + client_rect.bottom ) / 2;
					cursor_pos.x = center.x;
					cursor_pos.y = center.y;

					ClientToScreen( hwnd, &cursor_pos );
					SetCursorPos( cursor_pos.x, cursor_pos.y );
				}
			}
		}

	#elif defined( ORB_OS_LINUX )

		if( Window* window = Window::GetPtr(); window != nullptr )
		{
			auto&    details = window->GetPrivateDetails();
			XID      root_window;
			int32_t  x;
			int32_t  y;
			uint32_t width;
			uint32_t height;
			uint32_t border;
			uint32_t depth;
			
			if( XGetGeometry( details.display, details.window, &root_window, &x, &y, &width, &height, &border, &depth ) != 0 )
			{
				center.x = width / 2;
				center.y = height / 2;
			
				if( fps_cursor.enabled )
				{
					XWarpPointer( details.display, None, details.window, 0, 0, 0, 0, center.x, center.y );
					XFlush( details.display );
				}
			}
		}

	#elif defined( ORB_OS_MACOS )

		if( Window* window = Window::GetPtr(); window != nullptr )
		{
			auto&   details    = window->GetPrivateDetails();
			CGSize  size       = details.window.frame.size;
			CGPoint cursor_pos = CGPointMake( size.width / 2, size.height / 2 );

			center.x = lroundf( cursor_pos.x );
			center.y = lroundf( cursor_pos.y );

			CGWarpMouseCursorPosition( cursor_pos );
		}

	#endif

		for( auto& it : key_states )
		{
			it.second.pressed  = false;
			it.second.released = false;
		}

		for( auto& it : button_states )
		{
			it.second.pressed  = false;
			it.second.released = false;
		}

		for( auto& it : pointers )
		{
			it.second.previous_pos = it.second.current_pos;
		}
	}
}

ORB_NAMESPACE_END
