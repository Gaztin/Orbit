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

#include "UIWindow.h"

#if defined( ORB_OS_IOS )
#  include "Orbit/Core/Input/Input.h"
#  include "Orbit/Core/Widget/Point.h"
#  include "Orbit/Core/Widget/Window.h"

@implementation ORB_NAMESPACED_OBJC( UIWindow )

-( void )layoutSubviews
{
	[ super layoutSubviews ];

	ORB_NAMESPACE WindowResized e;
	e.width  = self.bounds.size.width;
	e.height = self.bounds.size.height;

	ORB_NAMESPACE Window::Get().QueueEvent( e );
}

-( void )touchesBegan:( NSSet< UITouch* >* )touches withEvent:( UIEvent* )event
{
	for( UITouch* touch in touches )
	{
		CGPoint   point = [ touch locationInView:self ];
		NSInteger index = [ touch.estimationUpdateIndex integerValue ];

		ORB_NAMESPACE Input::SetPointerPressed( index, ORB_NAMESPACE Point( point.x, point.y ) );
	}
}

-( void )touchesMoved:( NSSet< UITouch* >* )touches withEvent:( UIEvent* )event
{
	for( UITouch* touch in touches )
	{
		CGPoint   point = [ touch locationInView:self ];
		NSInteger index = [ touch.estimationUpdateIndex integerValue ];

		ORB_NAMESPACE Input::SetPointerPos( index, ORB_NAMESPACE Point( point.x, point.y ) );
	}
}

-( void )touchesEnded:( NSSet< UITouch* >* )touches withEvent:( UIEvent* )event
{
	for( UITouch* touch in touches )
	{
		CGPoint   point = [ touch locationInView:self ];
		NSInteger index = [ touch.estimationUpdateIndex integerValue ];

		ORB_NAMESPACE Input::SetPointerReleased( index, ORB_NAMESPACE Point( point.x, point.y ) );
	}
}

-( void )touchesCancelled:( NSSet< UITouch* >* )touches withEvent:( UIEvent* )event
{
	for( UITouch* touch in touches )
	{
		CGPoint   point = [ touch locationInView:self ];
		NSInteger index = [ touch.estimationUpdateIndex integerValue ];

		ORB_NAMESPACE Input::SetPointerReleased( index, ORB_NAMESPACE Point( point.x, point.y ) );
	}
}

@end

#endif // ORB_OS_IOS
