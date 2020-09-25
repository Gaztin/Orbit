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
#include "Orbit/Core/Core.h"

// Fix macro interference on Windows
#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#  undef RGB
#endif // ORB_OS_WINDOWS

ORB_NAMESPACE_BEGIN

class RGB
{
public:

	constexpr RGB( void ) = default;

	constexpr explicit RGB( float grey )
		: r( grey )
		, g( grey )
		, b( grey )
	{
	}

	constexpr RGB( float red, float green, float blue )
		: r( red )
		, g( green )
		, b( blue )
	{
	}

public:

	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

};

ORB_NAMESPACE_END
