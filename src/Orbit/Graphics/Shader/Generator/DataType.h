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

#include <string_view>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	enum class DataType
	{
		Unknown = 0,

		Float,
		Vec2,
		Vec3,
		Vec4,
		Mat4,
	};

	constexpr std::string_view DataTypeToString( DataType type )
	{
		switch( type )
		{
			case DataType::Float: { return "float";      }
			case DataType::Vec2:  { return "vec2";       }
			case DataType::Vec3:  { return "vec3";       }
			case DataType::Vec4:  { return "vec4";       }
			case DataType::Mat4:  { return "mat4";       }
			default:              { return "error_type"; }
		}
	}
}

ORB_NAMESPACE_END
