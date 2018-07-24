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

#include "window_impl.h"

#include <windows.h>

namespace orb
{

constexpr LPCSTR className = "Orbit";

ATOM window_impl::s_class = window_impl::create_window_class();

window_impl::window_impl()
	: m_open(false)
	, m_hwnd(create_window(CW_USEDEFAULT, CW_USEDEFAULT))
{
	m_open = (m_hwnd != nullptr);
}

window_impl::window_impl(uint32_t width, uint32_t height)
	: m_open(false)
	, m_hwnd(create_window(width, height))
{
	m_open = (m_hwnd != nullptr);
}

window_impl::~window_impl()
{
	DestroyWindow(m_hwnd);
}

void window_impl::poll_events()
{
	MSG msg;
	while (PeekMessageA(&msg, m_hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void window_impl::set_title(const std::string& title)
{
	SetWindowTextA(m_hwnd, title.c_str());
}

void window_impl::set_pos(uint32_t x, uint32_t y)
{
	RECT rect;
	GetWindowRect(m_hwnd, &rect);
	MoveWindow(m_hwnd, x, y, (rect.right - rect.left), (rect.top - rect.bottom), true);
}

void window_impl::set_visible(bool visible)
{
	ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);
}

ATOM window_impl::create_window_class()
{
	WNDCLASSEXA classDesc { };
	classDesc.cbSize        = sizeof(WNDCLASSEXA);
	classDesc.style         = CS_VREDRAW | CS_HREDRAW;
	classDesc.lpfnWndProc   = &window_impl::wnd_proc;
	classDesc.hInstance     = GetModuleHandleA(nullptr);
	classDesc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	classDesc.lpszClassName = className;

	/* Extract and copy icon from application. */
	char path[MAX_PATH + 1];
	GetModuleFileNameA(nullptr, path, sizeof(path));
	classDesc.hIcon = ExtractIconA(classDesc.hInstance, path, 0);

	return RegisterClassExA(&classDesc);
}

LRESULT window_impl::wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	window_impl& impl = *reinterpret_cast<window_impl*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
	switch (msg)
	{
		case WM_CLOSE:
			impl.close();
			break;

		default:
			break;
	}

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

HWND window_impl::create_window(int width, int height)
{
	HWND hwnd = CreateWindowExA(
		WS_EX_OVERLAPPEDWINDOW,
		className,
		nullptr,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		GetModuleHandleA(nullptr),
		nullptr);

	/* Set window user data. */
	SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	return hwnd;
}

}
