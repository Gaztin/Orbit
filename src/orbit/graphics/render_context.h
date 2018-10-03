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
#include "orbit/core/bitmask.h"
#include "orbit/core/variant.h"
#include "orbit/core/window.h"
#include "orbit/graphics.h"

namespace orb
{

class window;

class ORB_API_GRAPHICS render_context : public variant<80>
{
public:
	render_context(window& parentWindow, graphics_api api);

	void make_current(const window& parentWindow);
	void swap_buffers(const window& parentWindow);
	void clear(buffer_mask bm);
	void set_clear_color(float r, float g, float b);

private:
	graphics_api m_api;
	std::shared_ptr<window::subscription> m_windowSubscription;
};

}
