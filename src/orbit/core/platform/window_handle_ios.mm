/*
 * Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include <UIKit/UIKit.h>

#include "orbit/core/window.h"

@interface ORBWindow : UIWindow
@property ( nonatomic ) orb::event_dispatcher< orb::window_event >* eventDispatcher;
@end

namespace orb
{
	namespace platform
	{

		window_handle create_window_handle( uint32_t /*width*/, uint32_t /*height*/ )
		{
			window_handle wh = { };
			wh.uiWindow = [ ORBWindow alloc ];
			[ ( ORBWindow* )wh.uiWindow initWithFrame:[ [ UIScreen mainScreen ] bounds ] ];
			( ( ORBWindow* )wh.uiWindow ).backgroundColor = [ UIColor whiteColor ];
			[ ( ORBWindow* )wh.uiWindow makeKeyAndVisible ];

			UIViewController* vc = [ UIViewController alloc ];
			[ vc initWithNibName:nil bundle:nil ];
			( ( ORBWindow* )wh.uiWindow ).rootViewController = vc;

			return wh;
		}

		void set_window_user_data( window_handle& wh, window& wnd )
		{
			[ ( ORBWindow* )wh.uiWindow setEventDispatcher:&wnd ];
		}

		std::optional< message > peek_message( const window_handle& /*wh*/ )
		{
			return std::nullopt;
		}

		void process_message( window& /*wnd*/, const message& /*msg*/ )
		{
		}

		void set_window_title( const window_handle& /*wh*/, const std::string& /*title*/ )
		{
		}

		void set_window_position( const window_handle& /*wh*/, int /*x*/, int /*y*/ )
		{
		}

		void set_window_size( const window_handle& /*wh*/, uint32_t /*width*/, uint32_t /*height*/ )
		{
		}

		void set_window_visibility(const window_handle& wh, bool visible)
		{
			[ ( ORBWindow* )wh.uiWindow setHidden:!visible ];
		}
	}
}

@implementation ORBWindow

- ( void )layoutSubviews
{
	[ super layoutSubviews ];

	orb::window_event e = { };
	e.type = orb::window_event::Resize;
	e.data.resize.w = static_cast< uint32_t >( self.bounds.size.width );
	e.data.resize.h = static_cast< uint32_t >( self.bounds.size.height );
	_eventDispatcher->queue_event( e );
}

@end
