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
#include <string>

#include "orbit/core/event_dispatcher.h"
#include "orbit/core/variant.h"

namespace orb
{

struct window_event;

class ORB_API_CORE window : public variant<64>, public event_dispatcher<window_event>
{
public:
	window();
	window(uint32_t width, uint32_t height);

	void poll_events();
	void set_title(const std::string& title);
	void set_pos(uint32_t x, uint32_t y);
	void set_size(uint32_t width, uint32_t height);
	void show();
	void hide();

	operator bool() const;
};

struct window_event
{
	enum class type_t
	{
		None = 0,
		Move,
		Resize,
	};
	union data_t
	{
		struct move_t { float x, y; } move;
		struct resize_t { float w, h; } resize;
	};

	type_t type;
	data_t data;
};

}
