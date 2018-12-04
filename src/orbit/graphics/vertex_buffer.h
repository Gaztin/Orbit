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
#include <cstdlib>
#include <type_traits>

#include "orbit/graphics/platform/buffer_base.h"

namespace orb
{

class ORB_API_GRAPHICS vertex_buffer
{
public:
	vertex_buffer(const void* data, size_t count, size_t size);

	template <typename Vertex,
		typename = typename std::enable_if_t<std::is_object_v<Vertex>>>
	vertex_buffer(std::initializer_list<Vertex> vertices)
		: vertex_buffer(vertices.begin(), vertices.size(), sizeof(Vertex))
	{
	}

	void bind();

private:
	std::unique_ptr<platform::buffer_base> m_base;
};

}
