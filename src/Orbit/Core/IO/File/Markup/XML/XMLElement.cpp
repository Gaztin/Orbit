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

#include "XMLElement.h"

ORB_NAMESPACE_BEGIN

std::string_view XMLElement::FindAttribute( std::string_view attribute_name ) const
{
	for( const XMLAttribute& attribute : attributes )
	{
		if( attribute.name == attribute_name )
			return attribute.value;
	}

	return { };
}

const XMLElement* XMLElement::FindChild( std::string_view child_name ) const
{
	for( const XMLElement& child : children )
	{
		if( child.name == child_name )
			return &child;
	}

	return nullptr;
}

const XMLElement* XMLElement::FindChildWithAttribute( std::string_view child_name, XMLAttributeView attribute ) const
{
	for( const XMLElement& child : children )
	{
		if( child.name == child_name )
		{
			for( const XMLAttribute& attrib : child.attributes )
			{
				if( attrib.name == attribute.name && attrib.value == attribute.value )
					return &child;
			}
		}
	}

	return nullptr;
}

size_t XMLElement::CountChildrenWithName( std::string_view child_name ) const
{
	size_t count = 0;

	for( const XMLElement& child : children )
	{
		if( child.name == child_name )
			++count;
	}

	return count;
}

const XMLElement& XMLElement::operator[]( std::string_view child_name ) const
{
	for( const XMLElement& child : children )
	{
		if( child.name == child_name )
			return child;
	}

	static XMLElement dummy;
	return dummy;
}

ORB_NAMESPACE_END
