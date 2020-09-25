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
#include "Orbit/Core/IO/File/Markup/XML/XMLElement.h"
#include "Orbit/Core/IO/File/TextFile.h"

#include <vector>

ORB_NAMESPACE_BEGIN

class ORB_API_CORE XMLFile : public TextFile
{
public:

	/** Parses @data into an element hierarchy */
	explicit XMLFile( ByteSpan data );
	        ~XMLFile( void ) = default;

public:

	const XMLElement& GetRootElement( void ) const { return root_element_; }

protected:

	XMLElement root_element_;

private:

	/** Reads a string from @src, compatible as an XML element name, and returns the string */
	std::string ReadName( const char* src );

	/** Reads a string from @src, compatible as XML element content, and returns the string */
	std::string ReadContent( const char* src );

	/** Recursively parse elements from @src and insert them into @parent
	 * Returns false if the data was ill-formed, and true if the parser succeeded to parse the data into @parent */
	bool ParseElement( const char* src, XMLElement* parent );

};

ORB_NAMESPACE_END
