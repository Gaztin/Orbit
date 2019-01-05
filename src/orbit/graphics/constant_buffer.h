/*
* Copyright (c) 2019 Sebastian Kylander http://gaztin.com/
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
#include <memory>
#include <tuple>

#include "orbit/graphics/platform/constant_buffer_base.h"
#include "orbit/graphics/shader_constant.h"

namespace orb
{

class ORB_API_GRAPHICS constant_buffer
{
public:
	constant_buffer(size_t size);

	template<typename... Types>
	constant_buffer(const std::tuple<Types...>&)
		: constant_buffer((0 + ... + sizeof(Types)))
	{
	}

	void bind(shader_type type, uint32_t slot);
	void update(const void* data, size_t size);

	template<typename... Types>
	void update(const std::tuple<Types...>& constants)
	{
		update((sizeof...(Types) > 0 ? &std::get<0>(constants) : nullptr), (0 + ... + sizeof(Types)));
	}

private:
	std::unique_ptr<platform::constant_buffer_base> m_base;
};

}
