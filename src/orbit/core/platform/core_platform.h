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

#pragma once
#include "orbit/core.h"

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>

#include "orbit/core/platform/message.h"
#include "orbit/core/platform/window_handle.h"

namespace orb
{
class window;

namespace platform
{

#if defined(ORB_OS_ANDROID)
using argv_t= andrid_app*;
#else
using argv_t = std::initializer_list<char*>;
#endif

extern ORB_API_CORE window_handle create_window_handle(uint32_t width = 0, uint32_t height = 0);
extern ORB_API_CORE void set_window_user_data(window_handle& wh, window& wnd);
extern ORB_API_CORE std::optional<message> peek_message(const window_handle& wh);
extern ORB_API_CORE void process_message(window& wnd, const message& msg);
extern ORB_API_CORE void set_window_title(const window_handle& wh, const std::string& title);
extern ORB_API_CORE void set_window_position(const window_handle& wh, int x, int y);
extern ORB_API_CORE void set_window_size(const window_handle& wh, uint32_t width, uint32_t height);
extern ORB_API_CORE void set_window_visibility(const window_handle& wh, bool visible);

}
}
