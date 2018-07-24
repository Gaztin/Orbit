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

#include "window.h"

#include "orbit/core/internal/window_impl.h"

namespace orb
{

window::window()
{
	new (&impl<window_impl>()) window_impl();
}

window::window(uint32_t width, uint32_t height)
{
	new (&impl<window_impl>()) window_impl(width, height);
}

window::~window()
{
	impl<window_impl>().~window_impl();
}

void window::poll_events()
{
	impl<window_impl>().poll_events();
}

void window::set_title(const std::string& title)
{
	impl<window_impl>().set_title(title);
}

void window::set_pos(uint32_t x, uint32_t y)
{
	impl<window_impl>().set_pos(x, y);
}

void window::show()
{
	impl<window_impl>().set_visible(true);
}

void window::hide()
{
	impl<window_impl>().set_visible(false);
}

window::operator bool() const
{
	return impl<window_impl>().is_open();
}

}
