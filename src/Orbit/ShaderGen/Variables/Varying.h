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
#include "Orbit/Graphics/Geometry/VertexLayout.h"
#include "Orbit/ShaderGen/Variables/Variable.h"

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	template< VertexComponent VC, size_t RI >
	class VaryingHelper;

	class ORB_API_SHADERGEN Varying : public Variable
	{
	public:

		template< size_t RI > using Position     = VaryingHelper< VertexComponent::Position,     RI >;
		template< size_t RI > using Normal       = VaryingHelper< VertexComponent::Normal,       RI >;
		template< size_t RI > using Binormal     = VaryingHelper< VertexComponent::Binormal,     RI >;
		template< size_t RI > using Tangent      = VaryingHelper< VertexComponent::Tangent,      RI >;
		template< size_t RI > using Color        = VaryingHelper< VertexComponent::Color,        RI >;
		template< size_t RI > using TexCoord     = VaryingHelper< VertexComponent::TexCoord,     RI >;
		template< size_t RI > using BlendIndices = VaryingHelper< VertexComponent::BlendIndices, RI >;
		template< size_t RI > using BlendWeights = VaryingHelper< VertexComponent::BlendWeights, RI >;
		using Variable::operator=;
	
	public:
	
		Varying( VertexComponent component, size_t resource_index );

	private:
	
		std::string GetValueDerived( void ) const override;

	private:

		size_t resource_index_;
	
	};
	
	template< VertexComponent VC, size_t RI >
	class VaryingHelper : public Varying
	{
	public:
	
		using Varying::operator=;
	
	public:
	
		VaryingHelper( void )
			: Varying( VC, RI )
		{
		}
	
	};
}

ORB_NAMESPACE_END
