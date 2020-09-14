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
#include "Orbit/Core/IO/File/Markup/XML/XMLAttribute.h"

#include <string>
#include <vector>

ORB_NAMESPACE_BEGIN

class ORB_API_CORE XMLElement
{
public:

	/** Searches for an attribute with the name @name and returns its value */
	std::string_view FindAttribute( std::string_view name ) const;

	/** Searched for a child by the name @name */
	const XMLElement* FindChild( std::string_view name ) const;

public:

	/** Searches all children and returns a pointer to the first one with an element by the name @name that
	 * contains the attribute @attribute.
	 * Returns nullptr if no child was found.
	 */
	const XMLElement* FindChildWithAttribute( std::string_view name, XMLAttributeView attribute ) const;

	/** Returns the number of children called @name */
	size_t CountChildrenWithName( std::string_view name ) const;

public:

	/** Enable range-based looping over this element's children */
	auto begin ( void ) const { return children.begin(); }
	auto end   ( void ) const { return children.end(); }

public:

	/** Returns a reference to a child with the name @key.
	 * If no child was found, returns a reference to an empty dummy.
	 */
	const XMLElement& operator[]( std::string_view name ) const;

public:

	std::string                 name;
	std::string                 content;
	std::vector< XMLAttribute > attributes;
	std::vector< XMLElement >   children;

};

ORB_NAMESPACE_END
