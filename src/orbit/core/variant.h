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
#include <assert.h>
#include <stdint.h>

#include "orbit/core/utility.h"

namespace orb
{

template<size_t size>
class variant
{
public:
	variant()
		: m_storage{}
		, m_deleter(nullptr)
	{ }

	template<typename T, typename... Args> variant(in_place_type_t<T>, Args&&... args)
	{
		construct<T, Args...>(std::forward<Args>(args)...);
	}

	~variant()
	{
		m_deleter(m_storage);
	}

	template<typename T, typename... Args>
	T& construct(Args&&... args)
	{
		assert(sizeof(T) <= size);
		m_deleter = &delete_impl<T>;
		return *(new (m_storage) T(std::forward<Args>(args)...));
	}

	template<typename T>
	T& ref()
	{
		return *reinterpret_cast<T*>(m_storage);
	}

	template<typename T>
	const T& ref() const
	{
		return *reinterpret_cast<const T*>(m_storage);
	}

private:
	template<typename T>
	static void delete_impl(uint8_t* ptr)
	{
		reinterpret_cast<T*>(ptr)->~T();
	}

	uint8_t m_storage[size];

	void(*m_deleter)(uint8_t*);
};

}
