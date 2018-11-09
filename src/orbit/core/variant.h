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
#include <memory>

#include <stdint.h>

#include "orbit/core/utility.h"

namespace orb
{

class variant
{
public:
	variant()
		: m_ptr(nullptr)
	{ }

	template<typename T, typename... Args>
	variant(in_place_type_t<T>, Args&&... args)
	{
		construct<T, Args...>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	void construct(Args&&... args)
	{
		m_ptr = std::static_pointer_cast<void, T>(std::make_shared<T>(std::forward<Args>(args)...));
	}

	template<typename T>
	T& get()
	{
		return *reinterpret_cast<T*>(m_ptr.get());
	}

	template<typename T>
	const T& get() const
	{
		return *reinterpret_cast<const T*>(m_ptr.get());
	}

	template<typename T>
	std::shared_ptr<T> get_ptr() const
	{
		return std::static_pointer_cast<T, void>(m_ptr);
	}

private:
	std::shared_ptr<void> m_ptr;
};

}