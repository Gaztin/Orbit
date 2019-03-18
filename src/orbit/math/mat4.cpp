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

#include "mat4.h"

#include "orbit/math/vec3.h"
#include "orbit/math/vec4.h"

namespace orb
{

mat4::mat4()
	: m_elements{}
{
	for (size_t i = 0; i < 16; i += 5)
		m_elements[i] = 1.f;
}

mat4::mat4(std::initializer_list<float> elements)
{
	const size_t size = std::min(16u, elements.size());
	for (size_t i = 0; i < size; ++i)
	{
		m_elements[i] = *(elements.begin() + i);
	}
}

void mat4::translate(const vec3& t)
{
	const vec4 t4(t[0], t[1], t[2], 1.f);
	for (size_t i = 0; i < 4; ++i)
		m_elements[3 * 4 + i] = vec4(m_elements[0 * 4 + i], m_elements[1 * 4 + i], m_elements[2 * 4 + i], m_elements[3 * 4 + i]).dot_product(t4);
}

mat4 mat4::operator*(const mat4& rhs) const
{
	mat4 result;

	std::array<vec4, 4> columns;
	for (size_t i = 0; i < 4; ++i)
		columns[i] = vec4(m_elements[i * 4 + 0], m_elements[i * 4 + 1], m_elements[i * 4 + 2], m_elements[i * 4 + 3]);

	std::array<vec4, 4> rows;
	for (size_t i = 0; i < 4; ++i)
		rows[i] = vec4(rhs.m_elements[0 * 4 + i], rhs.m_elements[1 * 4 + i], rhs.m_elements[2 * 4 + i], rhs.m_elements[3 * 4 + i]);

	for (size_t row = 0; row < 4; ++row)
		for (size_t col = 0; col < 4; ++col)
			result.m_elements[row * 4 + col] = columns[row].dot_product(rows[col]);

	return result;
}

}
