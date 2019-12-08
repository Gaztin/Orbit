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

#include "GLSL.h"

ORB_NAMESPACE_BEGIN

namespace GLSL
{
	std::string_view GetVersionDirective( OpenGLVersion version )
	{
		if( version.IsEmbedded() )
		{
			/**/ if( version.RequireGLES( 3, 2 ) ) return "#version 320 es\n";
			else if( version.RequireGLES( 3, 0 ) ) return "#version 300 es\n";
			else                                   return "#version 100\n";
		}
		else
		{
			/**/ if( version.RequireGL( 4, 3 ) ) return "#version 430\n";
			else if( version.RequireGL( 4, 2 ) ) return "#version 420\n";
			else if( version.RequireGL( 4, 1 ) ) return "#version 410\n";
			else if( version.RequireGL( 4, 0 ) ) return "#version 400\n";
			else if( version.RequireGL( 3, 3 ) ) return "#version 330\n";
			else if( version.RequireGL( 3, 2 ) ) return "#version 150\n";
			else if( version.RequireGL( 3, 1 ) ) return "#version 140\n";
			else if( version.RequireGL( 3, 0 ) ) return "#version 130\n";
			else if( version.RequireGL( 2, 1 ) ) return "#version 120\n";
			else                                 return "#version 110\n";
		}
	}

	std::string_view GetGLSLDefine( void )
	{
		return "#define GLSL 1\n";
	}

	std::string_view GetShaderTypeDefine( ShaderType shader_type )
	{
		switch( shader_type )
		{
			default:                   { return "#error Invalid shader type\n"; } break;
			case ShaderType::Vertex:   { return "#define VERTEX 1\n";           } break;
			case ShaderType::Fragment: { return "#define FRAGMENT 1\n";         } break;
		}
	}

	std::string_view GetPrecision( OpenGLVersion version )
	{
		return version.IsEmbedded() ? "precision highp float;\n" : "";
	}

	std::string_view GetConstantsMacros( OpenGLVersion version )
	{
		/* GL 3.1+ or GLES 3 supports uniform buffer objects */
		if( version.RequireGL( 3 ) || version.RequireGLES( 3, 1 ) )
		{
			return "#define ORB_CONSTANTS_BEGIN(X) layout (std140) uniform X {\n#define ORB_CONSTANTS_END };\n#define ORB_CONSTANT(T, N) T N\n";
		}

		return "#define ORB_CONSTANTS_BEGIN(X)\n#define ORB_CONSTANTS_END\n#define ORB_CONSTANT(T, N) uniform T N\n";
	}

	std::string_view GetVaryingMacro( OpenGLVersion version, ShaderType shader_type )
	{
		if( version.RequireGL( 3, 3 ) || version.RequireGLES( 3 ) )
		{
			switch( shader_type )
			{
				default:                   return "";
				case ShaderType::Vertex:   return "#define ORB_VARYING out\n";
				case ShaderType::Fragment: return "#define ORB_VARYING in\n";
			}
		}

		return "#define ORB_VARYING varying\n";
	}

	std::string_view GetAttributeMacro( OpenGLVersion version, ShaderType shader_type )
	{
		if( version.RequireGL( 3, 3 ) || version.RequireGLES( 3 ) )
		{
			switch( shader_type )
			{
				default:                 return "";
				case ShaderType::Vertex: return "#define ORB_ATTRIBUTE( INDEX ) layout( location = INDEX ) in\n";
			}
		}

		return "#define ORB_ATTRIBUTE( INDEX ) attribute\n";
	}

	std::string_view GetOutColorMacro( OpenGLVersion version )
	{
		/* 'gl_FragColor' was deprecated in GL 3.0 and GLES 3 and replaced with output variables */
		if( version.RequireGL( 3, 3 ) || version.RequireGLES( 3 ) )
		{
			return "out vec4 _orb_outColor;\n#define ORB_SET_OUT_COLOR(X) _orb_outColor = X\n";
		}

		return "#define ORB_SET_OUT_COLOR(X) gl_FragColor = X\n";
	}
}

ORB_NAMESPACE_END
