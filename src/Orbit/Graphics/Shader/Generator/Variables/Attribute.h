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
#include "Orbit/Graphics/Shader/Generator/Variables/IVariable.h"
#include "Orbit/Graphics/Shader/VertexLayout.h"

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	template< VertexComponent VC >
	class AttributeHelper;

	class ORB_API_GRAPHICS Attribute : public IVariable
	{
	public:

		using Position = AttributeHelper< VertexComponent::Position >;
		using Normal   = AttributeHelper< VertexComponent::Normal >;
		using Color    = AttributeHelper< VertexComponent::Color >;
		using TexCoord = AttributeHelper< VertexComponent::TexCoord >;
		using IVariable::operator=;

	public:

		Attribute( VertexComponent component );

	public:

		std::string GetValue( void ) const override;

	};

	template< VertexComponent VC >
	class AttributeHelper : public Attribute
	{
	public:

		AttributeHelper( void )
			: Attribute( VC )
		{
		}

	};
}

ORB_NAMESPACE_END