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

#include <Cocoa/Cocoa.h>

namespace orb
{
namespace this_platform
{

@interface window_delegate : NSObject<NSWindowDelegate>
@property window* windowPtr;
- (void)windowWillClose:(NSNotification*)notification;
@end

@implementation window_delegate
- (void)windowWillClose:(NSNotification*)notification
{
	if (windowPtr)
		windowPtr->close();
}
@end

window_handle create_window_handle(uint32_t width, uint32_t height)
{
	window_handle handle;

	handle.nsWindow = [NSWindow alloc];
	[handle.nsWindow
		initWithContentRect:NSMakeRect(0.0f, 0.0f, width, height)
		styleMask:(NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
		backing:NSBackingStoreBuffered
		defer:NO];

	handle.delegate = [window_delegate alloc];
	[handle.nsWindow setDelegate:handle.delegate];

	return handle;
}

void set_window_user_data(window_handle& wh, window& wnd)
{
	[wh.delegate setWindowPtr:&wnd];
}

std::optional<message_t> peek_message(const window_handle& wh)
{
	message_t msg = [wh.nsWindow nextEventMatchingMask:NSEventMaskAny untilDate:nullptr inMode:NSDefaultRunLoopMode dequeue:YES];
	if (msg != nullptr)
		return msg;
	else
		return std::nullopt;
}

void process_message(window& wnd, const message_t& msg)
{
	switch ([msg type])
	{
		default:
			break;
	}
}

void set_window_title(const window_handle& wh, const std::string& title)
{
	NSString* nsTitle = [NSString stringWithUTF8String:title.c_str()];
	[wh.nsWindow setTitle:nsTitle];
	[nsTitle release];
}

void set_window_position(const window_handle& wh, int x, int y)
{
	NSRect frame = [wh.nsWindow frame];
	frame.origin.x = x;
	frame.origin.y = y;
	[wh.nsWindow setFrame:frame display:YES];
}

void set_window_size(const window_handle& wh, uint32_t width, uint32_t height)
{
	NSRect frame = [wh.nsWindow frame];
	frame.size.width  = width;
	frame.size.height = height;
	[wh.nsWindow setFrame:frame display:YES];
}

void set_window_visibility(const window_handle& wh, bool visible)
{
	[wh.nsWindow setIsVisible:visible];
}

}
}
