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
#elif defined(ORB_OS_ANDROID)
#include <android_native_app_glue.h>
#elif defined(ORB_OS_LINUX)
#include <X11/Xlib.h>
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

#if defined(ORB_OS_WINDOWS)
	HDC hdc() const { return m_hdc; }
	
#elif defined(ORB_OS_LINUX)
	Display* display() const { return m_display; }
	Window window() const { return m_window; }
	
#elif defined(ORB_OS_MACOS)
	void* window() const { return m_window; }
#endif

private:
#if defined(ORB_OS_WINDOWS)
	static ATOM create_window_class();
	static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND create_window(int width, int height);
	HDC format_device_context();

	static ATOM s_class;

	HWND m_hwnd;
	HDC  m_hdc;

#elif defined(ORB_OS_ANDROID)
	static void app_cmd(android_app* state, int cmd);
	static int input_event(android_app* state, AInputEvent* e);

#elif defined(ORB_OS_LINUX)
	Window create_xwindow(int width, int height) const;
	void set_delete_protocol() const;
	void process_xevent(const XEvent& e);

	Display* m_display;
	Window   m_window;
	
#elif defined(ORB_OS_MACOS)
	void* create_nswindow(int width, int height) const;
	void* create_delegate();
	void process_nsevent(void* e);
	
	void* m_window;
	void* m_delegate;
#endif

	bool m_open;
};

}
