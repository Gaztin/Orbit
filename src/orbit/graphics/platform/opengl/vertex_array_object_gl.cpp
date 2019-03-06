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

#include "vertex_array_object_gl.h"

#include "orbit/graphics/platform/opengl/gl.h"

namespace orb
{
namespace platform
{

vertex_array_object_gl::vertex_array_object_gl()
{
	orb::gl::get_current_functions().gen_vertex_arrays(1, &m_id);
}

vertex_array_object_gl::~vertex_array_object_gl()
{
	orb::gl::get_current_functions().delete_vertex_arrays(1, &m_id);
}

void vertex_array_object_gl::bind()
{
	orb::gl::get_current_functions().bind_vertex_array(m_id);
}

void vertex_array_object_gl::unbind()
{
	orb::gl::get_current_functions().bind_vertex_array(0);
}

}
}
