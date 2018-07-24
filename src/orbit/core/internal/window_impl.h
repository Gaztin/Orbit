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
#include "orbit.h"

#include <string>

#if defined(ORB_OS_WINDOWS)
#include <windows.h>
#endif

namespace orb
{

class ORB_DLL_LOCAL window_impl
{
public:
	window_impl();
	window_impl(uint32_t width, uint32_t height);
	~window_impl();

	void poll_events();
	void set_title(const std::string& title);
	void set_pos(uint32_t x, uint32_t y);
	void set_visible(bool visible);

	inline void close() { m_open = false; }
	inline bool is_open() const { return m_open; }

private:
#if defined(ORB_OS_WINDOWS)
	static ATOM create_window_class();
	static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND create_window(int width, int height);

	static ATOM s_class;

	HWND m_hwnd;

#endif

	bool m_open;
};

}
