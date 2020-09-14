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
#include "Orbit/Core/IO/File/BinaryFile.h"

ORB_NAMESPACE_BEGIN

class ORB_API_CORE TextFile : public BinaryFile
{
protected:

	/** Skips any character considered whitespace */
	void SkipWhitespace( const char* src );

	/** Peeks the characters in @src and returns whether they match @str and increments the offset */
	bool ExpectString( const char* src, std::string_view str );

	/** Reads characters from @src until it uncovers one that isn't alphanumeric (see @std::isalnum), at which point
	 * it returns all characters read as one string
	 */
	std::string ReadAlphaNumeric( const char* src );

	/** Reads characters from @src until it uncovers one that isn't printable (see @std::isprint), at which point
	 * it returns all characters read as one string
	 */
	std::string ReadPrintable( const char* src );

	/** Expects a leading quotation mark and reads characters until a matching trailing quotation mark is uncovered and
	 * returns all characters between the two quotation marks as one string
	 */
	std::string ReadCapturedStringLiteral( const char* src );

	/** Reads @length amount of characters from @src without moving the offset and returns them as one string */
	std::string Peek( const char* src, size_t length ) const;

	/** Search in @src for the character @c and return the number of characters traversed */
	size_t FindCharacter( const char* src, char c ) const;

};

ORB_NAMESPACE_END
