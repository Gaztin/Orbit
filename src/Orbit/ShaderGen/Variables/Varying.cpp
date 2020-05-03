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

#include "Orbit/ShaderGen/Generator/IShader.h"
#include "Orbit/ShaderGen/Generator/MainFunction.h"
#include "Orbit/ShaderGen/Generator/ShaderManager.h"

#include <cassert>
#include <sstream>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	Varying::Varying( VertexComponent component )
		: Variable( ShaderManager::GetInstance().NewVarying( component ), DataTypeFromVertexComponent( component ) )
	{
		stored_ = true;
	}

	std::string Varying::GetValueDerived( void ) const
	{
		if( ShaderManager::GetInstance().GetLanguage() == ShaderLanguage::HLSL )
		{
			switch( ShaderManager::GetInstance().GetType() )
			{
				case ShaderType::Vertex:   { return "output." + value_; }
				case ShaderType::Fragment: { return "input."  + value_; }
				default:                   { assert( false ); } break;
			}
		}

		return value_;
	}
}

ORB_NAMESPACE_END
