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
		Pos current;
		Pos previous;
	};

	static std::map< Key, KeyState >   key_states;
	static std::map< size_t, Pointer > pointers;

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

	void SetPointerPos( size_t index, Pos pos )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			it->second.previous = it->second.current;
			it->second.current  = pos;
		}
		else
		{
			Pointer& pointer = pointers[ index ];

			pointer.current  = pos;
			pointer.previous = pos;
		}
	}

	Pos GetPointerPos( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			return it->second.current;
		}

		return { };
	}

	Pos GetPointerMove( size_t index )
	{
		if( auto it = pointers.find( index ); it != pointers.end() )
		{
			const auto& [ curr_x, curr_y ] = it->second.current;
			const auto& [ prev_x, prev_y ] = it->second.previous;

			return std::make_tuple( curr_x - prev_x, curr_y - prev_y );
		}

		return { };
	}

	void ResetStates( void )
	{
		for( auto& it : key_states )
		{
			it.second.pressed  = false;
			it.second.released = false;
		}

		for( auto& it : pointers )
		{
			const auto& [ curr_x, curr_y ] = it.second.current;
			auto&       [ prev_x, prev_y ] = it.second.previous;

			prev_x = curr_x;
			prev_y = curr_y;
		}
	}
}

ORB_NAMESPACE_END
