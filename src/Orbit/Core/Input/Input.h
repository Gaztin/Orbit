/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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
#include "Orbit/Core/Input/Key.h"
#include "Orbit/Core/Utility/Singleton.h"
#include "Orbit/Core/Widget/Point.h"

#include <map>
#include <tuple>

ORB_NAMESPACE_BEGIN

namespace Input
{
	struct PointerIterator
	{
		PointerIterator& operator++( void );
		bool             operator!=( PointerIterator other ) const;
		size_t           operator* ( void )                  const;

		size_t index;
	};

	struct PointerIndices
	{
		PointerIterator begin( void ) const;
		PointerIterator end  ( void ) const;
	};

	extern ORB_API_CORE void           SetKeyPressed     ( Key key );
	extern ORB_API_CORE void           SetKeyReleased    ( Key key );
	extern ORB_API_CORE bool           GetKeyPressed     ( Key key );
	extern ORB_API_CORE bool           GetKeyReleased    ( Key key );
	extern ORB_API_CORE bool           GetKeyHeld        ( Key key );
	extern ORB_API_CORE void           SetPointerPressed ( size_t index, Point pos );
	extern ORB_API_CORE void           SetPointerReleased( size_t index, Point pos );
	extern ORB_API_CORE void           SetPointerPos     ( size_t index, Point pos );
	extern ORB_API_CORE bool           GetPointerPressed ( size_t index );
	extern ORB_API_CORE bool           GetPointerReleased( size_t index );
	extern ORB_API_CORE bool           GetPointerHeld    ( size_t index );
	extern ORB_API_CORE Point          GetPointerPos     ( size_t index );
	extern ORB_API_CORE Point          GetPointerMove    ( size_t index );
	extern ORB_API_CORE PointerIndices GetPointerIndices ( void );
	extern ORB_API_CORE void           SetFPSCursor      ( bool enable );
	extern ORB_API_CORE void           ResetStates       ( void );

	constexpr size_t pointer_index_mouse_left    = 0;
	constexpr size_t pointer_index_mouse_right   = 1;
	constexpr size_t pointer_index_mouse_middle  = 2;
	constexpr size_t pointer_index_mouse_extra_1 = 3;
	constexpr size_t pointer_index_mouse_extra_2 = 4;
};

ORB_NAMESPACE_END
