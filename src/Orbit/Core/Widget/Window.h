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

#pragma once
#include <string_view>

#include "Orbit/Core/Impl/WindowImpl.h"
#include "Orbit/Core/Event/CommonEvents.h"
#include "Orbit/Core/Event/EventDispatcher.h"

ORB_NAMESPACE_BEGIN

class ORB_API_CORE Window
{
public:
	enum class State
	{
		Focus,
		Defocus,
		Suspend,
		Restore,
		Close,
	};

	Window( uint32_t width, uint32_t height, WindowAPI api = kDefaultWindowApi );
	~Window();

	void PollEvents();
	void SetTitle( std::string_view title );
	void SetPos( uint32_t x, uint32_t y );
	void SetSize( uint32_t width, uint32_t height );
	void Show();
	void Hide();
	void Close() { m_open = false; }

	ORB_IMPL_SUBSCRIBE_FUNCTION( SubscribeToResize,       m_resize_dispatch );
	ORB_IMPL_SUBSCRIBE_FUNCTION( SubscribeToMove,         m_move_dispatch );
	ORB_IMPL_SUBSCRIBE_FUNCTION( SubscribeToStateChanged, m_state_dispatch );

	operator bool() const { return m_open; }

	WindowImpl* GetImplPtr() { return &m_impl; }

private:
	WindowImpl m_impl;
	bool       m_open;

	/* Event dispatchers */
	EventDispatcher< ResizeEvent >                m_resize_dispatch;
	EventDispatcher< MoveEvent >                  m_move_dispatch;
	EventDispatcher< StateChangedEvent< State > > m_state_dispatch;

};

ORB_NAMESPACE_END
