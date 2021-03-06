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
#include "Orbit/Core/Event/EventSubscription.h"
#include "Orbit/Graphics/Private/FrameBufferDetails.h"
#include "Orbit/Graphics/Texture/Texture2D.h"

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS FrameBuffer
{
public:

	 FrameBuffer( void );
	~FrameBuffer( void );

public:

	void Clear ( void );
	void Bind  ( void );
	void Unbind( void );

public:

	Texture2D& GetTexture2D( void ) { return texture2d_; }

private:

	void Resize( uint32_t width, uint32_t height );

private:

	Private::FrameBufferDetails framebuffer_details_;
	Texture2D                   texture2d_;
	EventSubscription           on_resize_;

};

ORB_NAMESPACE_END
