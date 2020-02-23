/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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
#include "Orbit/Graphics/Graphics.h"

#if( ORB_HAS_OPENGL )
#  include "Orbit/Core/Utility/Bitmask.h"

#  include <cstddef>
#  include <string_view>
#  include <type_traits>
#  include <utility>

ORB_NAMESPACE_BEGIN

using GLintptr   = ptrdiff_t;
using GLsizeiptr = size_t;
using GLdouble   = double;
using GLchar     = char;
using GLint64    = int64_t;
using GLsync     = struct __GLsync*;

extern ORB_API_GRAPHICS void* GetOpenGLProcAddress( std::string_view name );
extern ORB_API_GRAPHICS void  HandleOpenGLError   ( GLenum err, std::string_view func );

#define ORB_GL_CLAMP_TO_EDGE     0x812F
#define ORB_GL_DEPTH24_STENCIL8  0x88F0

ORB_NAMESPACE_END

#endif // ORB_HAS_OPENGL
