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

#include "core_platform.h"

namespace orb
{
namespace this_platform
{

window_handle create_window_handle(uint32_t width, uint32_t height)
{
	window_handle h;
	h.display = XOpenDisplay(nullptr);

	const int            screen  = DefaultScreen(h.display);
	XSetWindowAttributes attribs = { };
	handle.window = XCreateWindow(
		h.display,
		XRootWindow(h.display, screen),
		0,
		0,
		width,
		height,
		0,
		DefaultDepth(h.display, screen),
		InputOutput,
		DefaultVisual(h.display, screen),
		CWBackPixel,
		&attribs
	);

	/* Override the close event by including the DELETE_WINDOW atom in the protocol. */
	Atom atom = XInternAtom(h.display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(h.display, h.window, &atom, 1);

	return h;
}

void set_window_user_data(window_handle& /*wh*/, window& /*wnd*/)
{
}

std::optional<message_t> peek_message(const window_handle& wh)
{
	if (XPending(wh.display))
	{
		message_t msg;
		XNextEvent(wh.display, &msg);
		return msg;
	}
	else
	{
		return std::nullopt;
	}
}

void process_message(window& wnd, const message_t& msg)
{
	switch (msg.type)
	{
		case ClientMessage:
			wnd.close();
			break;
		default:
			break;
	}
}

void set_window_title(const window_handle& wh, const std::string& title)
{
	XStoreName(wh.display, wh.window, title.c_str());
}

void set_window_position(const window_handle& wh, int x, int y)
{
	XMoveWindow(wh.display, wh.window, x, y);
}

void set_window_size(const window_handle& wh, uint32_t width, uint32_t height)
{
	XResizeWindow(wh.display, wh.window, width, height);
}

void set_window_visibility(const window_handle& wh, bool visible)
{
	if (visible)
		XMapWindow(wh.display, wh.window);
	else
		XUnmapWindow(wh.display, wh.window);
}

}
}
