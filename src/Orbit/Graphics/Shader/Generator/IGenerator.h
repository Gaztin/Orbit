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
#include "Orbit/Core/Utility/StringLiteral.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Vec4.h"
#include "Orbit/Graphics/Shader/Generator/Swizzle.h"
#include "Orbit/Graphics/Shader/VertexLayout.h"

#include <string_view>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	class  UniformBase;
	struct ShaderCode;

	class ORB_API_GRAPHICS IGenerator
	{
		friend class Attribute;
		friend class Sampler;
		friend class UniformBase;
		friend class Varying;

	public:

		IGenerator( void );
		virtual ~IGenerator( void );

	public:

		std::string  Generate       ( void );
		VertexLayout GetVertexLayout( void ) const;

	public:

		static IGenerator* GetCurrentGenerator ( void );
		static ShaderCode* GetCurrentShaderCode( void );

	protected:

		virtual Vec4 VSMain( void ) = 0;
		virtual Vec4 PSMain( void ) = 0;

	protected:

		IVariable Transpose( const IVariable& matrix );
		IVariable Sample   ( const IVariable& sampler, const IVariable& texcoord );
		IVariable Dot      ( const IVariable& lhs, const IVariable& rhs );

	private:

		std::string GenerateHLSL( void );
		std::string GenerateGLSL( void );

	private:

		std::vector< UniformBase* > m_uniforms;
		VertexLayout                m_attribute_layout;
		VertexLayout                m_varying_layout;
		uint32_t                    m_sampler_count;

	};
}

ORB_NAMESPACE_END
