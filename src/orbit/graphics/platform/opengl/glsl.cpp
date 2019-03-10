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

#include "glsl.h"

namespace orb
{
	namespace glsl
	{
		std::string_view get_version_directive( const version& glVersion, bool embedded )
		{
			if( embedded )
			{
				/**/ if( glVersion >= version( 3, 2 ) ) return "#version 320 es\n";
				else if( glVersion >= version( 3, 0 ) ) return "#version 300 es\n";
				else                                    return "#version 100\n";
			}
			else
			{
				/**/ if( glVersion >= version( 4, 3 ) ) return "#version 430\n";
				else if( glVersion >= version( 4, 2 ) ) return "#version 420\n";
				else if( glVersion >= version( 4, 1 ) ) return "#version 410\n";
				else if( glVersion >= version( 4, 0 ) ) return "#version 400\n";
				else if( glVersion >= version( 3, 3 ) ) return "#version 330\n";
				else if( glVersion >= version( 3, 2 ) ) return "#version 150\n";
				else if( glVersion >= version( 3, 1 ) ) return "#version 140\n";
				else if( glVersion >= version( 3, 0 ) ) return "#version 130\n";
				else if( glVersion >= version( 2, 1 ) ) return "#version 120\n";
				else                                    return "#version 110\n";
			}
		}

		std::string_view get_glsl_define( void )
		{
			return "#define ORB_GLSL 1\n";
		}

		std::string_view get_precision( bool embedded )
		{
			return embedded ? "precision highp float;\n" : "";
		}

		std::string_view get_constants_macros( const version& glVersion, bool embedded )
		{
			/* GLES 3 or GL 3.1+ supports uniform buffer objects */
			if( ( embedded && glVersion >= version( 3 ) ) || ( !embedded && glVersion >= version( 3, 1 ) ) )
				return "#define ORB_CONSTANTS_BEGIN(X) layout (std140) uniform X {\n#define ORB_CONSTANTS_END };\n#define ORB_CONSTANT(T, N) T N\n";

			return "#define ORB_CONSTANTS_BEGIN(X)\n#define ORB_CONSTANTS_END\n#define ORB_CONSTANT(T, N) uniform T N\n";
		}

		std::string_view get_varying_macro( const version& glVersion, bool embedded, shader_type type )
		{
			if( ( embedded && glVersion >= version( 3 ) ) || ( !embedded && glVersion >= version( 3, 3 ) ) )
			{
				switch( type )
				{
					default:                    return "";
					case shader_type::Vertex:   return "#define ORB_VARYING out\n";
					case shader_type::Fragment: return "#define ORB_VARYING in\n";
				}
			}

			return "#define ORB_VARYING varying\n";
		}

		std::string_view get_attribute_macro( const version& glVersion, bool embedded, shader_type type )
		{
			if( ( embedded && glVersion >= version( 3 ) ) || ( !embedded && glVersion >= version( 3, 3 ) ) )
			{
				switch( type )
				{
					default:                  return "";
					case shader_type::Vertex: return "#define ORB_ATTRIBUTE in\n";
				}
			}

			return "#define ORB_ATTRIBUTE attribute\n";
		}

		std::string_view get_out_color_macro( const version& glVersion, bool embedded )
		{
			/* 'gl_FragColor' was deprecated in GLES 3 and GL 3.0 and replaced with output variables */
			if( ( embedded && glVersion >= version( 3 ) ) || ( !embedded && glVersion >= version( 3, 3 ) ) )
				return "out vec4 _orb_outColor;\n#define ORB_SET_OUT_COLOR(X) _orb_outColor = X\n";

			return "#define ORB_SET_OUT_COLOR(X) gl_FragColor = X\n";
		}
	}
}