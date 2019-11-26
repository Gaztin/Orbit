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
#include <cstdint>
#include <type_traits>

#include "Orbit/Core/Core.h"

ORB_NAMESPACE_BEGIN

struct ORB_API_CORE ResizeEvent
{
	uint32_t width;
	uint32_t height;
};

struct ORB_API_CORE MoveEvent
{
	int x;
	int y;
};

template< typename T,
	typename = typename std::enable_if_t< std::is_enum_v< T > > >
struct ORB_API_CORE StateChangedEvent
{
	T value;
};

ORB_NAMESPACE_END
