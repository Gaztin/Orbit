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

#include "Clock.h"

#include <chrono>

ORB_NAMESPACE_BEGIN

static std::chrono::high_resolution_clock::time_point start;
static std::chrono::high_resolution_clock::time_point then;
static std::chrono::high_resolution_clock::time_point now;

float Clock::GetLife( void )
{
	auto life_time = std::chrono::duration_cast< std::chrono::duration< float > >( now - start );
	return life_time.count();
}

float Clock::GetDelta( void )
{
	auto delta_time = std::chrono::duration_cast< std::chrono::duration< float > >( now - then );
	return delta_time.count();
}

void Clock::Start( void )
{
	start = std::chrono::high_resolution_clock::now();
	then  = start;
	now   = start;
}

void Clock::Update( void )
{
	then = now;
	now  = std::chrono::high_resolution_clock::now();
}

ORB_NAMESPACE_END
