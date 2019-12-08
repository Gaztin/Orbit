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
#include <string_view>

#include "Orbit/Core/Utility/Version.h"
#include "Orbit/Graphics/Graphics.h"

ORB_NAMESPACE_BEGIN

namespace GLSL
{
	extern ORB_API_GRAPHICS std::string_view GetVersionDirective ( const Version& version, bool embedded );
	extern ORB_API_GRAPHICS std::string_view GetGLSLDefine       ( void );
	extern ORB_API_GRAPHICS std::string_view GetShaderTypeDefine ( ShaderType shader_type );
	extern ORB_API_GRAPHICS std::string_view GetPrecision        ( bool embedded );
	extern ORB_API_GRAPHICS std::string_view GetConstantsMacros  ( const Version& version, bool embedded );
	extern ORB_API_GRAPHICS std::string_view GetVaryingMacro     ( const Version& version, bool embedded, ShaderType shader_type );
	extern ORB_API_GRAPHICS std::string_view GetAttributeMacro   ( const Version& version, bool embedded, ShaderType shader_type );
	extern ORB_API_GRAPHICS std::string_view GetOutColorMacro    ( const Version& version, bool embedded );
}

ORB_NAMESPACE_END
