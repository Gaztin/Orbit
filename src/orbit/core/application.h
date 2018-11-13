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
#include <type_traits>

#include "orbit/core/platform/main.h"

namespace orb
{

class application
{
public:
	application() = default;
	virtual ~application() = default;

	virtual void frame() {}
	virtual operator bool() const { return true; };

	template<typename T,
		typename = typename std::enable_if_t<std::is_base_of_v<application, T>>>
	static void main(platform::argv_t argv)
	{
		platform::main(argv, []() -> std::unique_ptr<application> { return std::make_unique<T>(); });
	}
};

}
