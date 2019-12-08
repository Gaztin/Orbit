/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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

#include "WindowDelegate.h"

#if defined( ORB_OS_MACOS )
#  include "Orbit/Core/Widget/Window.h"

@implementation ORB_NAMESPACED_OBJC( WindowDelegate )

-( void )windowWillClose:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE Window::GetInstance().Close();
}

-( void )windowDidMove:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE Window& window  = ORB_NAMESPACE Window::GetInstance();
	auto&                 details = window.GetPrivateDetails();
	const CGPoint         point   = details.window.frame.origin;

	ORB_NAMESPACE WindowMoved e;
	e.x = point.x;
	e.y = point.y;

	window.QueueEvent( e );
}

-( NSSize )windowWillResize:( NSWindow* ) __unused sender toSize:( NSSize ) frame_size
{
	ORB_NAMESPACE WindowResized e;
	e.width  = frame_size.width;
	e.height = frame_size.height;

	Window::GetInstance().QueueEvent( e );

	return frame_size;
}

-( void )windowDidMiniaturize:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowStateChanged e;
	e.state = ORB_NAMESPACE WindowState::Suspend;

	Window::GetInstance().QueueEvent( e );
}

-( void )windowDidDeminiaturize:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowStateChanged e;
	e.state = ORB_NAMESPACE WindowState::Restore;

	Window::GetInstance().QueueEvent( e );
}

-( void )windowDidBecomeMain:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowStateChanged e;
	e.state = ORB_NAMESPACE WindowState::Focus;

	Window::GetInstance().QueueEvent( e );
}

-( void )windowDidResignMain:( NSNotification* ) __unused notification
{
	ORB_NAMESPACE WindowStateChanged e;
	e.state = ORB_NAMESPACE WindowState::Defocus;

	Window::GetInstance().QueueEvent( e );
}

@end

#endif
