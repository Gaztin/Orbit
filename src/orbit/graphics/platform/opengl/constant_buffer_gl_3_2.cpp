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

#include "constant_buffer_gl_3_2.h"

#include <cstddef>
#include <cstring>

#include "orbit/graphics/platform/opengl/gl.h"
#include "orbit/graphics/platform/opengl/render_context_gl.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
	namespace platform
	{
		constant_buffer_gl_3_2::constant_buffer_gl_3_2( size_t size )
			: m_id( 0 )
		{
			auto& gl = gl::get_current_functions();
			gl.gen_buffers( 1, &m_id );
			gl.bind_buffer( gl::buffer_target::Uniform, m_id );
			gl.buffer_data( gl::buffer_target::Uniform, size, nullptr, orb::gl::buffer_usage::StreamDraw );
			gl.bind_buffer( gl::buffer_target::Uniform, 0 );
		}

		constant_buffer_gl_3_2::~constant_buffer_gl_3_2()
		{
			gl::get_current_functions().delete_buffers( 1, &m_id );
		}

		void constant_buffer_gl_3_2::update( size_t /*location*/, const void* data, size_t size )
		{
			auto& gl = gl::get_current_functions();;
			gl.bind_buffer( gl::buffer_target::Uniform, m_id );
			void* dst = gl.map_buffer_range( gl::buffer_target::Uniform, 0, size, gl::map_access::WriteBit );
			std::memcpy( dst, data, size );
			gl.unmap_buffer( gl::buffer_target::Uniform );
			gl.bind_buffer( gl::buffer_target::Uniform, 0 );
		}

		void constant_buffer_gl_3_2::bind( shader_type /*type*/, uint32_t slot )
		{
			auto& gl = gl::get_current_functions();;
			gl.bind_buffer( gl::buffer_target::Uniform, m_id );
			gl.bind_buffer_base( gl::buffer_target::Uniform, slot, m_id );
		}
	}
}
