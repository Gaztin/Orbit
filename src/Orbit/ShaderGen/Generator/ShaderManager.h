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
#include "Orbit/Core/Utility/Singleton.h"
#include "Orbit/Graphics/Geometry/VertexLayout.h"
#include "Orbit/ShaderGen/Variables/DataType.h"
#include "Orbit/ShaderGen/ShaderGen.h"

#include <sstream>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	class  UniformBase;
	struct MainFunction;

	class ORB_API_SHADERGEN ShaderManager : public Singleton< ShaderManager >
	{
		friend class IShader;

	public:

		std::ostringstream& Append      ( void ) const;
		std::string         NewLocal    ( DataType type, std::string_view code ) const;
		std::string         NewAttribute( VertexComponent component ) const;
		std::string         NewVarying  ( VertexComponent component ) const;
		std::string         NewSampler  ( void ) const;
		std::string         NewUniform  ( UniformBase* uniform ) const;
		ShaderLanguage      GetLanguage ( void ) const;
		ShaderType          GetType     ( void ) const;

	private:

		IShader*      current_shader_;
		MainFunction* current_main_function_;

	};
}

ORB_NAMESPACE_END
