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

#include <Cocoa/Cocoa.h>

#include "orbit/core/window.h"

@interface WindowDelegate : NSObject<NSWindowDelegate>
@property orb::window* windowPtr;
@end

namespace orb
{
namespace platform
{

window_handle create_window_handle(uint32_t width, uint32_t height)
{
	window_handle wh{};
	wh.nsWindow = [NSWindow alloc];
	[(NSWindow*)wh.nsWindow
		initWithContentRect:NSMakeRect(0.0f, 0.0f, width, height)
		styleMask:(NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
		backing:NSBackingStoreBuffered
		defer:NO];

	wh.delegate = [WindowDelegate alloc];
	[(NSWindow*)wh.nsWindow setDelegate:(WindowDelegate*)wh.delegate];

	return wh;
}

void set_window_user_data(window_handle& wh, window& wnd)
{
	[(WindowDelegate*)wh.delegate setWindowPtr:&wnd];
}

std::optional<message> peek_message(const window_handle& wh)
{
	message msg{};
	msg.nsEvent = [(const NSWindow*)wh.nsWindow nextEventMatchingMask:NSEventMaskAny untilDate:nullptr inMode:NSDefaultRunLoopMode dequeue:YES];
	if (msg.nsEvent != nullptr)
		return msg;
	else
		return std::nullopt;
}

void process_message(window& wnd, const message& msg)
{
	const window_handle& wh = wnd.get_handle();
	[(const NSWindow*)wh.nsWindow sendEvent:(NSEvent*)msg.nsEvent];
}

void set_window_title(const window_handle& wh, const std::string& title)
{
	NSString* nsTitle = [NSString stringWithUTF8String:title.c_str()];
	[(const NSWindow*)wh.nsWindow setTitle:nsTitle];
	[nsTitle release];
}

void set_window_position(const window_handle& wh, int x, int y)
{
	NSRect frame = [(const NSWindow*)wh.nsWindow frame];
	frame.origin.x = x;
	frame.origin.y = y;
	[(const NSWindow*)wh.nsWindow setFrame:frame display:YES];
}

void set_window_size(const window_handle& wh, uint32_t width, uint32_t height)
{
	NSRect frame = [(const NSWindow*)wh.nsWindow frame];
	frame.size.width  = width;
	frame.size.height = height;
	[(const NSWindow*)wh.nsWindow setFrame:frame display:YES];
}

void set_window_visibility(const window_handle& wh, bool visible)
{
	[(const NSWindow*)wh.nsWindow setIsVisible:visible];
}

}
}

@implementation WindowDelegate

- (void)windowWillClose:(NSNotification*)notification
{
	(void)notification;

	_windowPtr->close();
}

- (void)windowDidMove:(NSNotification*)notification
{
	(void)notification;

	const orb::platform::window_handle& wh = _windowPtr->get_handle();
	const CGPoint point = ((const NSWindow*)wh.nsWindow).frame.origin;

	orb::window_event e{};
	e.type = orb::window_event::Move;
	e.data.move.x = static_cast<int>(point.x);
	e.data.move.y = static_cast<int>(point.y);
	_windowPtr->queue_event(e);
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize
{
	(void)sender;

	orb::window_event e{};
	e.type = orb::window_event::Resize;
	e.data.resize.w = static_cast<uint32_t>(frameSize.width);
	e.data.resize.h = static_cast<uint32_t>(frameSize.height);
	_windowPtr->queue_event(e);

	return frameSize;
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
	(void)notification;

	orb::window_event e{};
	e.type = orb::window_event::Suspend;
	_windowPtr->queue_event(e);
}

- (void)windowDidDeminiaturize:(NSNotification*)notification
{
	(void)notification;

	orb::window_event e{};
	e.type = orb::window_event::Restore;
	_windowPtr->queue_event(e);
}

- (void)windowDidBecomeMain:(NSNotification*)notification
{
	(void)notification;
	
	orb::window_event e{};
	e.type = orb::window_event::Focus;
	_windowPtr->queue_event(e);
}

- (void)windowDidResignMain:(NSNotification*)notification
{
	(void)notification;

	orb::window_event e{};
	e.type = orb::window_event::Defocus;
	_windowPtr->queue_event(e);
}

@end
