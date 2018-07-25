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

namespace orb
{

window_impl::window_impl()
	: m_display(XOpenDisplay(nullptr))
	, m_window(create_xwindow(0, 0))
{
	m_open = (m_window != 0);
	set_delete_protocol();
}

window_impl::window_impl(uint32_t width, uint32_t height)
	: m_display(XOpenDisplay(nullptr))
	, m_window(create_xwindow(width, height))
{
	m_open = (m_window != 0);
	set_delete_protocol();
}

window_impl::~window_impl()
{
	XDestroyWindow(m_display, m_window);
	XCloseDisplay(m_display);
}

void window_impl::poll_events()
{
	XEvent e;
	while (XPending(m_display))
	{
		XNextEvent(m_display, &e);
		process_xevent(e);
	}
}

void window_impl::set_title(const std::string& title)
{
	XStoreName(m_display, m_window, title.c_str());
}

void window_impl::set_pos(uint32_t x, uint32_t y)
{
	XMoveWindow(m_display, m_window, x, y);
}

void window_impl::set_visible(bool visible)
{
	if (visible)
		XMapWindow(m_display, m_window);
	else
		XUnmapWindow(m_display, m_window);
}

Window window_impl::create_xwindow(int width, int height) const
{
	int screen = DefaultScreen(m_display);
	XSetWindowAttributes attribs { };
	return XCreateWindow(
		m_display,
		XRootWindow(m_display, screen),
		0,
		0,
		width,
		height,
		0,
		DefaultDepth(m_display, screen),
		InputOutput,
		DefaultVisual(m_display, screen),
		CWBackPixel,
		&attribs
	);
}

void window_impl::set_delete_protocol() const
{
	/* Override the close event by including the DELETE_WINDOW atom in the protocols. */
	Atom atom = XInternAtom(m_display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(m_display, m_window, &atom, 1);
}

void window_impl::process_xevent(const XEvent& e)
{
	switch (e.type)
	{
		case ClientMessage:
			m_open = false;
			break;
		
		default:
			break;
	}
}

}
