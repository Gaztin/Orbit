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

#include "vertex_buffer.h"

#include <assert.h>

#include "orbit/graphics/render_context.h"
#include "platform/d3d11/vertex_buffer_d3d11.h"
#include "platform/opengl/buffer_gl.h"

namespace orb
{
	static std::unique_ptr< platform::buffer_base > init_base( const void* data, size_t count, size_t stride )
	{
		switch( render_context::get_current()->get_api() )
		{
		#if defined( ORB_HAS_OPENGL )
			case graphics_api::OpenGL_2_0:
			case graphics_api::OpenGL_3_2:
			case graphics_api::OpenGL_4_1:
			case graphics_api::OpenGL_ES_2:
			case graphics_api::OpenGL_ES_3:
				return std::make_unique< platform::buffer_gl< gl::buffer_target::Array > >( data, count * stride );
			#endif

			#if defined( ORB_HAS_D3D11 )
			case graphics_api::Direct3D_11:
				return std::make_unique< platform::vertex_buffer_d3d11 >( data, count, stride );
			#endif

			default:
				return nullptr;
		}
	}

	vertex_buffer::vertex_buffer( const void* data, size_t count, size_t stride )
		: m_base( init_base( data, count, stride ) )
		, m_count( count )
	{
	}

	void vertex_buffer::bind()
	{
		m_base->bind();
	}
}
