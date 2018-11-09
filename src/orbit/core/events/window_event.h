/*
* Copyright (c) 2018 Sebastian Kylander http://gaztin.com/
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
#include <stdint.h>

#if defined(ORB_OS_LINUX)
// Need to temporarily undefine "None" since Xlib helped themselves to define it as a pre-processor.
#pragma push_macro("None")
#undef None
#endif

namespace orb
{

struct window_event
{
	enum
	{
		None = 0,
		Move,
		Resize,
		Defocus,
		Focus,
		Suspend,
		Restore,
		Close,
	} type;

	union
	{
		struct { int x, y; } move;
		struct { uint32_t w, h; } resize;
	} data;
};

}

#if defined(ORB_OS_LINUX)
#pragma pop_macro("None")
#endif