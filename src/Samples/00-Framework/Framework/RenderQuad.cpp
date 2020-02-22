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

#include "RenderQuad.h"

#include <Orbit/Math/Vector4.h>

#include <initializer_list>

static std::initializer_list< Orbit::Vector4 > vertices
{
	{ -1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, -1.0f, 0.0f, 1.0f },
	{ -1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f,  1.0f, 0.0f, 1.0f },
};

static std::initializer_list< uint16_t > indices
{
	0, 2, 1,
	3, 1, 2,
};

RenderQuad::RenderQuad( void )
	: vertex_buffer_( vertices )
	, index_buffer_ ( indices )
{
}