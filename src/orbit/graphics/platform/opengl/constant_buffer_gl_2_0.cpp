/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "constant_buffer_gl_2_0.h"

#include <cstddef>
#include <cstring>

#include "orbit/graphics/platform/opengl/gl.h"
#include "orbit/graphics/platform/opengl/render_context_gl.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
	namespace platform
	{
		void constant_buffer_gl_2_0::update( size_t location, const void* data, size_t /*size*/ )
		{
			auto& gl = gl::get_current_functions();
			// #TODO: won't work for any other uniform types other than single floats.
			gl.uniform1f( location, *reinterpret_cast< const GLfloat* >( data ) );
		}

		void constant_buffer_gl_2_0::bind( shader_type /*type*/, uint32_t /*slot*/ )
		{
		}
	}
}
