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
#include "Orbit/Graphics/Private/Texture2DDetails.h"

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS Texture2D
{
public:

	 Texture2D( uint32_t width, uint32_t height, const void* data, PixelFormat pixel_format );
	~Texture2D();

public:

	void Bind  ( uint32_t slot );
	void Unbind( uint32_t slot );

public:

	Private::Texture2DDetails&       GetPrivateDetails( void )       { return details_; }
	const Private::Texture2DDetails& GetPrivateDetails( void ) const { return details_; }

private:

	Private::Texture2DDetails details_;

};

ORB_NAMESPACE_END
