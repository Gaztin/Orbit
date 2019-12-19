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
#include <map>
#include <tuple>

#include "Orbit/Core/Input/Key.h"
#include "Orbit/Core/Utility/Singleton.h"
#include "Orbit/Core/Widget/Point.h"

ORB_NAMESPACE_BEGIN

namespace Input
{
	extern ORB_API_CORE void SetKeyPressed ( Key key );
	extern ORB_API_CORE void SetKeyReleased( Key key );
	extern ORB_API_CORE bool GetKeyPressed ( Key key );
	extern ORB_API_CORE bool GetKeyReleased( Key key );
	extern ORB_API_CORE bool GetKeyHeld    ( Key key );

	extern ORB_API_CORE void  SetPointerPressed ( size_t index, Point pos );
	extern ORB_API_CORE void  SetPointerReleased( size_t index, Point pos );
	extern ORB_API_CORE void  SetPointerPos     ( size_t index, Point pos );
	extern ORB_API_CORE bool  GetPointerPressed ( size_t index );
	extern ORB_API_CORE bool  GetPointerReleased( size_t index );
	extern ORB_API_CORE bool  GetPointerHeld    ( size_t index );
	extern ORB_API_CORE Point GetPointerPos     ( size_t index );
	extern ORB_API_CORE Point GetPointerMove    ( size_t index );

	extern ORB_API_CORE void SetFPSCursor( bool enable );

	extern ORB_API_CORE void ResetStates( void );
};

ORB_NAMESPACE_END
