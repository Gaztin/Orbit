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
#include <utility>

#include <stdint.h>

#include "orbit/core.h"

namespace orb
{

template<size_t width>
class ORB_DLL_LOCAL opaque
{
#if defined(ORB_BUILD)
public:
	template<typename T, typename... Args>
	void construct(Args&&... args)
	{
		static_assert(sizeof(T) <= width, "Specified layout too large");
		new (m_memory) T(std::forward<Args>(args)...);
	}

	template<typename T>
	void destruct()
	{
		static_assert(sizeof(T) <= width, "Specified layout too large");
		impl<T>().~T();
	}

	template<typename T>
	T& impl()
	{
		static_assert(sizeof(T) <= width, "Specified layout too large");
		return reinterpret_cast<T&>(*m_memory);
	}

	template<typename T>
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
