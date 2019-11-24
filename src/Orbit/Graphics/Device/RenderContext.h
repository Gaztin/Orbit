/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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
#include "Orbit/Core/Widget/Window.h"
#include "Orbit/Graphics/Impl/RenderContextImpl.h"

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS RenderContext
{
public:
	RenderContext( Window& parent_window, GraphicsAPI api = kDefaultGraphicsApi );
	~RenderContext();

	bool MakeCurrent();
	void Resize( uint32_t width, uint32_t height );
	void SwapBuffers();
	void Clear( BufferMask mask );
	void SetClearColor( float r, float g, float b );

	RenderContextImpl* GetImplPtr() { return &m_impl; }

	static RenderContext* GetCurrent();

private:
	RenderContextImpl       m_impl;
	Window::SubscriptionPtr m_resize_subscription;

};

ORB_NAMESPACE_END