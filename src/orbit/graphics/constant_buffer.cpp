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

#include "constant_buffer.h"

#include "orbit/graphics/platform/d3d11/constant_buffer_d3d11.h"
#include "orbit/graphics/platform/opengl/constant_buffer_gl_2_0.h"
#include "orbit/graphics/platform/opengl/constant_buffer_gl_3_2.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
	static std::unique_ptr< platform::constant_buffer_base > init_base( size_t size )
	{
		switch( render_context::get_current()->get_api() )
		{
		#if defined( ORB_HAS_OPENGL )
			case graphics_api::OpenGL_2_0:
			case graphics_api::OpenGL_ES_2:
				return std::make_unique< platform::constant_buffer_gl_2_0 >();

			case graphics_api::OpenGL_3_2:
			case graphics_api::OpenGL_4_1:
			case graphics_api::OpenGL_ES_3:
				return std::make_unique< platform::constant_buffer_gl_3_2 >( size );
		#endif

		#if defined( ORB_HAS_D3D11 )
			case graphics_api::Direct3D_11:
				return std::make_unique< platform::constant_buffer_d3d11 >( size );
		#endif

			default:
				return nullptr;
		}
	}

	constant_buffer::constant_buffer( size_t size )
		: m_base( init_base( size ) )
	{
	}

	void constant_buffer::update( size_t location, const void* data, size_t size )
	{
		m_base->update( location, data, size );
	}

	void constant_buffer::bind( shader_type type, uint32_t slot )
	{
		m_base->bind( type, slot );
	}
}
