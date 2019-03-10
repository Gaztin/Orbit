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
#include "orbit/math/internal/vec_base.h"

namespace orb
{

class vec4 : public internal::vec_base<4>
{
public:
	vec4() = default;

	explicit vec4(float scalar)
		: vec_base(scalar)
	{
	}

	vec4(float x, float y, float z, float w)
		: vec_base(x, y, z, w)
	{
	}

	float&       get_x()        { return m_elements[0]; }
	const float& get_x() const  { return m_elements[0]; }
	void         set_x(float x) { m_elements[0] = x; }

	float&       get_y()        { return m_elements[1]; }
	const float& get_y() const  { return m_elements[1]; }
	void         set_y(float y) { m_elements[1] = y; }

	float&       get_z()        { return m_elements[2]; }
	const float& get_z() const  { return m_elements[2]; }
	void         set_z(float z) { m_elements[2] = z; }

	float&       get_w()        { return m_elements[3]; }
	const float& get_w() const  { return m_elements[3]; }
	void         set_w(float w) { m_elements[3] = w; }
};

}
