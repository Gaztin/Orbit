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

#include <Cocoa/Cocoa.h>

@interface window_delegate : NSObject<NSWindowDelegate>
@property orb::window_impl* impl;
- (void)windowWillClose:(NSNotification*)notification;
@end

@implementation window_delegate
- (void)windowWillClose:(NSNotification*)notification
{
	_impl->close();
}
@end

namespace orb
{

window_impl::window_impl()
	: m_window(create_nswindow(0, 0))
	, m_delegate(create_delegate())
	, m_open(m_window != nullptr)
{
}

window_impl::window_impl(uint32_t width, uint32_t height)
	: m_window(create_nswindow(width, height))
	, m_delegate(create_delegate())
	, m_open(m_window != nullptr)
{
}

window_impl::~window_impl()
{
	[(window_delegate*)m_delegate dealloc];
	[(NSWindow*)m_window dealloc];
}

void window_impl::poll_events()
{
	NSEvent* e;
	while ((e = [(NSWindow*)m_window nextEventMatchingMask:NSEventMaskAny untilDate:nullptr inMode:NSDefaultRunLoopMode dequeue:YES]) != nullptr)
	{
		process_nsevent(e);
		[(NSWindow*)m_window sendEvent:e];
	}
}

void window_impl::set_title(const std::string& title)
{
	NSString* nsTitle = [NSString stringWithUTF8String:title.c_str()];
	[(NSWindow*)m_window setTitle:nsTitle];
	[nsTitle release];
}

void window_impl::set_pos(uint32_t x, uint32_t y)
{
	NSRect frame = [(NSWindow*)m_window frame];
	frame.origin.x = x;
	frame.origin.y = y;
	[(NSWindow*)m_window setFrame:frame display:YES];
}

void window_impl::set_visible(bool visible)
{
	[(NSWindow*)m_window setIsVisible:visible];
}

void* window_impl::create_nswindow(int width, int height) const
{
	NSWindow* window = [NSWindow alloc];
	[window
		initWithContentRect:NSMakeRect(0.0f, 0.0f, width, height)
		styleMask:(NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
		backing:NSBackingStoreBuffered
		defer:NO
	];
	return window;
}

void* window_impl::create_delegate()
{
	/* Create window delegate. */
	window_delegate* delegate = [window_delegate alloc];
	[delegate setImpl:this];
	[(NSWindow*)m_window setDelegate:delegate];
	return delegate;
}

void window_impl::process_nsevent(void* e)
{
	switch ([(NSEvent*)e type])
	{
		default:
			break;
	}
}

}
