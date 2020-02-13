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
#include "Orbit/Graphics/Shader/VertexLayout.h"

#include <string_view>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	enum class DataType
	{
		Unknown = 0,

		Float,
		FVec2,
		FVec3,
		FVec4,
		Mat4,
		Int,
		IVec2,
		IVec3,
		IVec4,

		Array,
	};

	constexpr std::string_view DataTypeToString( DataType type )
	{
		switch( type )
		{
			default:              { return "error_type"; }
			case DataType::Float: { return "float";      }
			case DataType::FVec2: { return "vec2";       }
			case DataType::FVec3: { return "vec3";       }
			case DataType::FVec4: { return "vec4";       }
			case DataType::Mat4:  { return "mat4";       }
			case DataType::Int:   { return "int";        }
			case DataType::IVec2: { return "ivec2";      }
			case DataType::IVec3: { return "ivec3";      }
			case DataType::IVec4: { return "ivec4";      }
		}
	}

	inline DataType DataTypeFromVertexComponent( VertexComponent component )
	{
		IndexedVertexComponent dummy{ component, 0 };

		switch( dummy.GetDataType() )
		{
			case PrimitiveDataType::Float:
			{
				switch( dummy.GetDataCount() )
				{
					case 1: return DataType::Float;
					case 2: return DataType::FVec2;
					case 3: return DataType::FVec3;
					case 4: return DataType::FVec4;
				}
			} break;

			case PrimitiveDataType::Int:
			{
				switch( dummy.GetDataCount() )
				{
					case 1: return DataType::Int;
					case 2: return DataType::IVec2;
					case 4: return DataType::IVec3;
					case 3: return DataType::IVec4;
				}
			} break;
		}

		return DataType::Unknown;
	}
}

ORB_NAMESPACE_END
