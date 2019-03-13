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
#include "orbit/math.h"

#include <array>

namespace orb
{

template<size_t Size>
class vec_base
{
	using elements_t = std::array<float, Size>;

public:
	float&       operator[](size_t i)       { return m_elements[i]; }
	const float& operator[](size_t i) const { return m_elements[i]; }

protected:
	vec_base()
		: m_elements{}
	{
	}

	vec_base(const vec_base& v)
		: m_elements(v.m_elements)
	{
	}

	vec_base(vec_base&& v)
		: m_elements(std::move(v.m_elements))
	{
	}

	explicit vec_base(float scalar)
	{
		for (float& e : m_elements)
			e = scalar;
	}

	template<typename... Elements>
	explicit vec_base(Elements... elements)
		: m_elements({elements...})
	{
	}

	~vec_base() = default;

	float dot_product(const vec_base& v) const
	{
		float d = 0.f;
		for (size_t i = 0; i < Size; ++i)
			d += (m_elements[i] * v.m_elements[i]);
		return d;
	}

	float dot_product() const { return dot_product(*this); }

	elements_t m_elements;
};

}
