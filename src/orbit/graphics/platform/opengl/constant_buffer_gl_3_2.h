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

#include <cstddef>

#include "orbit/graphics/platform/opengl/gl.h"
#include "orbit/graphics/platform/constant_buffer_base.h"

namespace orb
{
	namespace platform
	{
		class constant_buffer_gl_3_2 : public constant_buffer_base
		{
		public:
			constant_buffer_gl_3_2( size_t size );
			~constant_buffer_gl_3_2();

			void update ( size_t location, const void* data, size_t size ) final override;
			void bind   ( shader_type type, uint32_t slot )                final override;

		private:
			GLuint m_id;
		};
	}
}
