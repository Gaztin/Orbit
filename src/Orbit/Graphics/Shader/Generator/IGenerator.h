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
#include "Orbit/Graphics/Shader/Generator/Variables/Attribute.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Sampler.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Uniform.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Varying.h"
#include "Orbit/Graphics/Shader/Generator/Swizzle.h"
#include "Orbit/Graphics/Shader/VertexLayout.h"

#include <string_view>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	struct MainFunction;

	class ORB_API_GRAPHICS IGenerator
	{
		friend class Variables::IVariable;
		friend class Variables::Attribute;
		friend class Variables::Sampler;
		friend class Variables::UniformBase;
		friend class Variables::Varying;

	public:

		IGenerator( void );
		virtual ~IGenerator( void );

	public:

		std::string  Generate       ( void );
		VertexLayout GetVertexLayout( void ) const;

	protected:

		using Sampler   = Variables::Sampler;
		using Attribute = Variables::Attribute;
		using Varying   = Variables::Varying;

		template< typename T >
		using Uniform = Variables::Uniform< T >;

		template< typename T, size_t N >
		using UniformArray = Variables::UniformArray< T, N >;

		using Float = Variables::Float;
		using Vec2  = Variables::Vec2;
		using Vec3  = Variables::Vec3;
		using Vec4  = Variables::Vec4;
		using Mat4  = Variables::Mat4;

	protected:

		virtual Vec4 VSMain( void ) = 0;
		virtual Vec4 PSMain( void ) = 0;

	protected:

		Variables::IVariable CanonicalScreenPos( const Variables::IVariable& pos );
		Variables::IVariable Transpose         ( const Variables::IVariable& matrix );
		Variables::IVariable Sample            ( const Variables::IVariable& sampler, const Variables::IVariable& texcoord );
		Variables::IVariable Dot               ( const Variables::IVariable& lhs, const Variables::IVariable& rhs );
		Variables::IVariable Normalize         ( const Variables::IVariable& vec );
		Variables::IVariable Cos               ( const Variables::IVariable& radians );
		Variables::IVariable Sin               ( const Variables::IVariable& radians );

	private:

		std::string GenerateHLSL( void );
		std::string GenerateGLSL( void );

	private:

		static IGenerator*   GetCurrentGenerator   ( void );
		static MainFunction* GetCurrentMainFunction( void );

	private:

		std::vector< Variables::UniformBase* > uniforms_;
		VertexLayout                           attribute_layout_;
		VertexLayout                           varying_layout_;
		uint32_t                               sampler_count_;

	};
}

ORB_NAMESPACE_END
