/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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
#include "Orbit/Core/Event/EventDispatcher.h"
#include "Orbit/Core/Private/WindowDetails.h"
#include "Orbit/Core/Utility/Singleton.h"

#include <string_view>

ORB_NAMESPACE_BEGIN

enum class WindowState
{
	Focus,
	Defocus,
	Suspend,
	Restore,
	Close,
};

struct ORB_API_CORE WindowResized
{
	uint32_t width;
	uint32_t height;
};

struct ORB_API_CORE WindowMoved
{
	int x;
	int y;
};

struct ORB_API_CORE WindowStateChanged
{
	WindowState state;
};

class ORB_API_CORE Window
	: public EventDispatcher< WindowResized, WindowMoved, WindowStateChanged >
	, public ManualSingleton< Window >
{
public:

	 Window( uint32_t width, uint32_t height );
	~Window( void );

public:

	void PollEvents( void );
	void SetTitle  ( std::string_view title );
	void Move      ( int32_t x, int32_t y );
	void Resize    ( uint32_t width, uint32_t height );
	void Show      ( void );
	void Hide      ( void );
	void Close     ( void );

public:

	Private::WindowDetails&       GetPrivateDetails( void )       { return details_; }
	const Private::WindowDetails& GetPrivateDetails( void ) const { return details_; }
	bool                          IsOpen           ( void ) const { return open_; }

private:

	Private::WindowDetails details_;
	bool                   open_;

};

ORB_NAMESPACE_END
