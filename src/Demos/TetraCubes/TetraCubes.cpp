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

#include "TetraCubes.h"

#include <Orbit/Core/Application/EntryPoint.h>

TetraCubes::TetraCubes( void )
	: window_ ( 768, 768 )
	, context_( Orbit::default_graphics_api )
{
	window_.Show();
	context_.SetClearColor( 0.1f, 0.1f, 0.1f );
}

void TetraCubes::OnFrame( float /*delta_time*/ )
{
	window_.PollEvents();

	context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );
	context_.SwapBuffers();
}

bool TetraCubes::IsRunning( void )
{
	return window_.IsOpen();
}
