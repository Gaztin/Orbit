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

#include "Varying.h"

#include "Orbit/Graphics/Shader/Generator/IGenerator.h"
#include "Orbit/Graphics/Shader/Generator/ShaderCode.h"

#include <cassert>
#include <sstream>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	static std::string NewName( void )
	{
		static uint32_t unique_index = 0;

		std::ostringstream ss;
		ss << "varying_" << ( unique_index++ );

		return ss.str();
	}

	static VariableType TypeOf( VertexComponent component )
	{
		switch( ( IndexedVertexComponent{ component, 0 } ).GetDataCount() )
		{
			default: { assert( false );            };
			case 1:  { return VariableType::Float; };
			case 2:  { return VariableType::Vec2;  };
			case 3:  { return VariableType::Vec3;  };
			case 4:  { return VariableType::Vec4;  };
		}
	}

	Varying::Varying( VertexComponent component )
		: IVariable( NewName(), TypeOf( component ) )
	{
		m_stored = true;

		IGenerator::GetCurrentGenerator()->m_varying_layout.Add( component );
	}

	std::string Varying::GetValue( void ) const
	{
		ShaderCode* code = IGenerator::GetCurrentShaderCode();

		if( code->language == ShaderLanguage::HLSL )
		{
			switch( code->type )
			{
				case ShaderType::Vertex:   { return "output." + m_value; }
				case ShaderType::Fragment: { return "input."  + m_value; }
				default:                   { assert( false ); } break;
			}
		}

		return m_value;
	}
}

ORB_NAMESPACE_END
