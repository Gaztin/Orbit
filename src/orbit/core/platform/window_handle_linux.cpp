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

#include "orbit/core/platform/window_handle.h"
#include "orbit/core/platform/message.h"
#include "orbit/core/window.h"

namespace orb
{
	namespace platform
	{
		window_handle create_window_handle( uint32_t width, uint32_t height )
		{
			window_handle wh = { };
			wh.display = XOpenDisplay( nullptr );

			const int screen = DefaultScreen( wh.display );
			XSetWindowAttributes attribs = { };
			attribs.event_mask = ( FocusChangeMask | ResizeRedirectMask | StructureNotifyMask );
			wh.window = XCreateWindow(
				wh.display,
				XRootWindow( wh.display, screen ),
				0,
				0,
				width,
				height,
				0,
				DefaultDepth( wh.display, screen ),
				InputOutput,
				DefaultVisual( wh.display, screen ),
				CWBackPixel | CWEventMask,
				&attribs
			);

			Atom closeAtom = XInternAtom( wh.display, "WM_DELETE_WINDOW", True );
			XSetWMProtocols( wh.display, wh.window, &close_atom, 1 );

			return wh;
		}

		void set_window_user_data( window_handle& /*wh*/, window& /*wnd*/ )
		{
		}

		std::optional< message > peek_message( const window_handle& wh )
		{
			if( XPending( wh.display ) )
			{
				message msg;
				XNextEvent( wh.display, &msg.xEvent );
				return msg;
			}
			else
			{
				return std::nullopt;
			}
		}

		void process_message( window& wnd, const message& msg )
		{
			switch( msg.xEvent.type )
			{
				case FocusIn:
				{
					if( msg.xEvent.xfocus.mode != NotifyNormal )
						break;

					window_event e = { };
					e.type = window_event::Focus;
					wnd.queue_event( e );
					break;
				}

				case FocusOut:
				{
					if( msg.xEvent.xfocus.mode != NotifyNormal )
						break;

					window_event e = { };
					e.type = window_event::Defocus;
					wnd.queue_event( e );
					break;
				}

				case ResizeRequest:
				{
					window_event e = { };
					e.type          = window_event::Resize;
					e.data.resize.w = msg.xEvent.xresizerequest.width;
					e.data.resize.h = msg.xEvent.xresizerequest.height;
					wnd.queue_event( e );
					break;
				}

				case ConfigureNotify:
				{
					window_event e = { };
					e.type        = window_event::Move;
					e.data.move.x = msg.xEvent.xconfigure.x;
					e.data.move.y = msg.xEvent.xconfigure.y;
					wnd.queue_event( e );
					break;
				}

				case ClientMessage:
				{
					wnd.close();
					break;
				}

				default:
					break;
			}
		}

		void set_window_title( const window_handle& wh, const std::string& title )
		{
			XStoreName( wh.display, wh.window, title.c_str() );
		}

		void set_window_position( const window_handle& wh, int x, int y )
		{
			XMoveWindow( wh.display, wh.window, x, y );
		}

		void set_window_size( const window_handle& wh, uint32_t width, uint32_t height )
		{
			XResizeWindow( wh.display, wh.window, width, height );
		}

		void set_window_visibility( const window_handle& wh, bool visible )
		{
			if( visible )
				XMapWindow( wh.display, wh.window );
			else
				XUnmapWindow( wh.display, wh.window );
		}
	}
}
