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
#include "Orbit/Graphics/Graphics.h"

#if defined( ORB_HAS_OPENGL )

#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Orbit/Core/Utility/Bitmask.h"

#if defined( Bool )
#  pragma push_macro( "Bool" )
#  undef Bool
#  define UNDEFINED_Bool
#endif

ORB_NAMESPACE_BEGIN

using GLintptr   = ptrdiff_t;
using GLsizeiptr = size_t;
using GLdouble   = double;
using GLchar     = char;
using GLint64    = int64_t;
using GLsync     = struct __GLsync*;

extern ORB_API_GRAPHICS void* GetOpenGLProcAddress( std::string_view name );
extern ORB_API_GRAPHICS void  HandleOpenGLError   ( GLenum err, std::string_view func );

ORB_NAMESPACE_END

#if defined( UNDEFINED_Bool )
#  pragma pop_macro( "Bool" )
#  undef UNDEFINED_Bool
#endif

#endif
