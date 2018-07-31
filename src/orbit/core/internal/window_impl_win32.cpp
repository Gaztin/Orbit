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

#include <assert.h>
#include <windows.h>

#include "orbit/core/utility.h"

namespace orb
{

constexpr LPCSTR className = "Orbit";

ATOM window_impl::s_class = window_impl::create_window_class();

window_impl::window_impl()
	: m_hwnd(create_window(CW_USEDEFAULT, CW_USEDEFAULT))
	, m_hdc(format_device_context())
	, m_open(m_hwnd != nullptr)
{
}

window_impl::window_impl(uint32_t width, uint32_t height)
	: m_hwnd(create_window(width, height))
	, m_hdc(format_device_context())
	, m_open(m_hwnd != nullptr)
{
}

window_impl::~window_impl()
{
	ReleaseDC(m_hwnd, m_hdc);
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
	classDesc.hbrBackground = cast<HBRUSH>(cast<LONG_PTR>(COLOR_WINDOW));
	classDesc.lpszClassName = className;

	/* Extract and copy icon from application. */
	char path[MAX_PATH + 1];
	GetModuleFileNameA(nullptr, path, sizeof(path));
	classDesc.hIcon = ExtractIconA(classDesc.hInstance, path, 0);

	return RegisterClassExA(&classDesc);
}

LRESULT window_impl::wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR userData = GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	switch (msg)
	{
		case WM_CLOSE:
			assert(userData);
			cast<window_impl*>(userData)->close();
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
	SetWindowLongPtrA(hwnd, GWLP_USERDATA, cast<LONG_PTR>(this));

	return hwnd;
}

HDC window_impl::format_device_context()
{
	HDC hdc = GetDC(m_hwnd);

	PIXELFORMATDESCRIPTOR pixelFormat { };
	pixelFormat.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
	pixelFormat.nVersion   = 1;
	pixelFormat.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormat.iPixelType = PFD_TYPE_RGBA;
	pixelFormat.cColorBits = 24;
	pixelFormat.cDepthBits = 24;
	pixelFormat.iLayerType = PFD_MAIN_PLANE;
	SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pixelFormat), &pixelFormat);

	return hdc;
}

}
