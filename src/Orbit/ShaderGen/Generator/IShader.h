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
#include "Orbit/Graphics/Geometry/VertexLayout.h"
#include "Orbit/ShaderGen/Variables/Attribute.h"
#include "Orbit/ShaderGen/Variables/Sampler.h"
#include "Orbit/ShaderGen/Variables/Swizzle.h"
#include "Orbit/ShaderGen/Variables/Uniform.h"
#include "Orbit/ShaderGen/Variables/Varying.h"

#include <string_view>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	struct MainFunction;

	class ORB_API_SHADERGEN IShader
	{
		friend class ShaderManager;

	public:

		IShader( void );
		virtual ~IShader( void );

	public:

		std::string  Generate       ( void );
		VertexLayout GetVertexLayout( void ) const;

	protected:

		using Sampler   = Sampler;
		using Attribute = Attribute;
		using Varying   = Varying;

		template< typename T >
		using Uniform = Uniform< T >;

		template< typename T, size_t N >
		using UniformArray = UniformArray< T, N >;

		using Float = Float;
		using Vec2  = Vec2;
		using Vec3  = Vec3;
		using Vec4  = Vec4;
		using Mat4  = Mat4;

	protected:

		virtual Vec4 VSMain( void ) = 0;
		virtual Vec4 PSMain( void ) = 0;

	protected:

		IVariable CanonicalScreenPos( const IVariable& pos );
		IVariable Transpose         ( const IVariable& matrix );
		IVariable Sample            ( const IVariable& sampler, const IVariable& texcoord );
		IVariable Dot               ( const IVariable& lhs, const IVariable& rhs );
		IVariable Normalize         ( const IVariable& vec );
		IVariable Cos               ( const IVariable& radians );
		IVariable Sin               ( const IVariable& radians );

	private:

		std::string GenerateHLSL( void );
		std::string GenerateGLSL( void );

	private:

		std::vector< UniformBase* > uniforms_;
		VertexLayout                attribute_layout_;
		VertexLayout                varying_layout_;
		uint32_t                    sampler_count_;

	};
}

ORB_NAMESPACE_END
