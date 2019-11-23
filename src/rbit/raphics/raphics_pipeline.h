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

#pragma once

#include "orbit/graphics/internal/graphics_pipeline_impl.h"

namespace orb
{
	class fragment_shader;
	class index_buffer;
	class vertex_buffer;
	class vertex_shader;

	class ORB_API_GRAPHICS graphics_pipeline
	{
	public:
		graphics_pipeline();
		~graphics_pipeline();

		void bind();
		void unbind();
		void set_shaders( const vertex_shader& vert, const fragment_shader& frag );
		void describe_vertex_layout( vertex_layout layout );

		void draw( const vertex_buffer& vb );
		void draw( const index_buffer& ib );

	private:
		graphics_pipeline_impl m_impl;

	};
}
