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

#include "Sampler.h"

#include "Orbit/Graphics/Shader/Generator/IGenerator.h"

#include <sstream>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	static std::string NewName( void )
	{
		static uint32_t unique_index = 0;

		std::ostringstream ss;
		ss << "sampler_" << ( unique_index++ );

		return ss.str();
	}

	Sampler::Sampler( void )
		: IVariable( NewName(), VariableType::Unknown )
	{
		m_stored = true;

		IGenerator::IncrementSamplerCount();
	}
}

ORB_NAMESPACE_END
