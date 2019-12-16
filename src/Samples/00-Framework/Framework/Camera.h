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
#include <cstdint>

#include <Orbit/Core/Event/EventSubscription.h>
#include <Orbit/Math/Matrix4.h>

namespace Orbit { struct WindowResized; }

class Camera
{
public:

	Camera( void );

public:

	void Update( float delta_time );

public:

	Orbit::Matrix4 GetViewProjection( void ) const;

public:

	float fov       = 60.0f;
	float near_clip = 0.1f;
	float far_clip  = 100.0f;
	float speed     = 4.0f;

private:

	void OnResized ( const Orbit::WindowResized& e );
	void UpdateView( float delta_time );

private:

	Orbit::EventSubscription m_on_resize;
	Orbit::Matrix4           m_view;

	uint32_t m_width  = 512;
	uint32_t m_height = 512;

};
