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

#include "window_handle.h"

#include <string>

#include <assert.h>
#include <windows.h>

#include "orbit/core/utility.h"
#include "orbit/core/window.h"

namespace orb
{
namespace platform
{

constexpr LPCSTR className = "Orbit";

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR userData = GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	if (userData == 0)
		return DefWindowProcA(hwnd, msg, wParam, lParam);

	window& wnd = *cast<window*>(userData);
	switch (msg)
	{
		case WM_MOVE:
			wnd.queue_event({window_event::Move, {LOWORD(lParam), HIWORD(lParam)}});
			break;

		case WM_SIZE:
			wnd.queue_event({window_event::Resize, {LOWORD(lParam), HIWORD(lParam)}});
			break;

		case WM_ACTIVATE:
			if (HIWORD(wParam) != 0)
				wnd.queue_event({(LOWORD(wParam) == WA_INACTIVE) ? window_event::Suspend : window_event::Restore});
			break;

		case WM_SETFOCUS:
			wnd.queue_event({window_event::Focus});
			break;

		case WM_KILLFOCUS:
			wnd.queue_event({window_event::Defocus});
			break;

		case WM_CLOSE:
			wnd.close();
			wnd.queue_event({window_event::Close});
			break;

		default:
			break;
	}

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static ATOM create_window_class()
{
	WNDCLASSEXA classDesc{};
	classDesc.cbSize        = sizeof(WNDCLASSEXA);
	classDesc.style         = CS_VREDRAW | CS_HREDRAW;
	classDesc.lpfnWndProc   = &wnd_proc;
	classDesc.hInstance     = GetModuleHandleA(nullptr);
	classDesc.hbrBackground = cast<HBRUSH>(cast<LONG_PTR>(COLOR_WINDOW));
	classDesc.lpszClassName = className;

	/* Extract and copy icon from application. */
	char path[MAX_PATH + 1];
	GetModuleFileNameA(nullptr, path, sizeof(path));
	classDesc.hIcon = ExtractIconA(classDesc.hInstance, path, 0);

	return RegisterClassExA(&classDesc);
}

window_handle create_window_handle(uint32_t width, uint32_t height)
{
	static ATOM wndClass = create_window_class();

	window_handle handle;
	handle.hwnd = CreateWindowExA(
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

	return handle;
}

void set_window_user_data(window_handle& wh, window& wnd)
{
	SetWindowLongPtrA(wh.hwnd, GWLP_USERDATA, cast<LONG_PTR>(&wnd));
}

std::optional<message> peek_message(const window_handle& wh)
{
	message msg;
	if (PeekMessageA(&msg.msg, wh.hwnd, 0, 0, PM_REMOVE))
		return msg;
	else
		return std::nullopt;
}

void process_message(window& /*wnd*/, const message& msg)
{
	TranslateMessage(&msg.msg);
	DispatchMessageA(&msg.msg);
}

void set_window_title(const window_handle& wh, const std::string& title)
{
	SetWindowTextA(wh.hwnd, title.c_str());
}

void set_window_position(const window_handle& wh, int x, int y)
{
	RECT rect{};
	GetWindowRect(wh.hwnd, &rect);
	MoveWindow(wh.hwnd, x, y, (rect.right - rect.left), (rect.bottom - rect.top), FALSE);
}

void set_window_size(const window_handle& wh, uint32_t width, uint32_t height)
{
	RECT rect{};
	GetWindowRect(wh.hwnd, &rect);
	MoveWindow(wh.hwnd, rect.left, rect.top, static_cast<int>(width), static_cast<int>(height), FALSE);
}

void set_window_visibility(const window_handle& wh, bool visible)
{
	ShowWindow(wh.hwnd, visible ? SW_SHOW : SW_HIDE);
}

}
}
