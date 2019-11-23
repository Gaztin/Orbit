/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "orbit/core/window.h"
#include "orbit/graphics/internal/render_context_impl.h"

namespace orb
{
	class ORB_API_GRAPHICS render_context
	{
	public:
		render_context( window& parentWindow, graphics_api api = kDefaultGraphicsApi );
		~render_context();

		bool make_current();
		void resize( uint32_t width, uint32_t height );
		void swap_buffers();
		void clear( buffer_mask mask );
		void set_clear_color( float r, float g, float b );

		render_context_impl* get_impl_ptr() { return &m_impl; }

		static render_context* get_current();

	private:
		render_context_impl      m_impl;
		window::subscription_ptr m_resizeSubscription;

	};
}