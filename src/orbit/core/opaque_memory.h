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

#include <utility>

#include <stdint.h>
#include <stdlib.h>

namespace orb
{

template<size_t width, typename T>
class ORB_DLL_LOCAL opaque_memory
{
#if defined(ORB_BUILD)
public:
	opaque_memory(const opaque_memory&) = default;
	opaque_memory(opaque_memory&&) = default;
	template<typename... Args>
	opaque_memory(Args&&... args) { new (m_memory) T(std::forward<Args>(args)...); }
	~opaque_memory() { impl().~T(); }

	T& impl()
	{
		static_assert(sizeof(T) <= width, "Specified layout too large");
		return reinterpret_cast<T&>(*m_memory);
	}

	const T& impl() const
	{
		static_assert(sizeof(T) <= width, "Specified layout too large");
		return reinterpret_cast<const T&>(*m_memory);
	}

#endif

private:
	uint8_t m_memory[width];
};

}
