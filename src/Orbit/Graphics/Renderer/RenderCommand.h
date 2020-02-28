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
#include "Orbit/Core/Utility/Ref.h"
#include "Orbit/Graphics/Graphics.h"

#include <map>
#include <vector>

ORB_NAMESPACE_BEGIN

class ConstantBuffer;
class FrameBuffer;
class IndexBuffer;
class Shader;
class Texture2D;
class VertexBuffer;

struct ORB_API_GRAPHICS RenderCommand
{
	std::map< ShaderType, std::vector< Ref< ConstantBuffer > > > constant_buffers;

	std::vector< Ref< Texture2D > > textures;

	Ref< VertexBuffer > vertex_buffer;
	Ref< IndexBuffer >  index_buffer;
	Ref< Shader >       shader;
	Ref< FrameBuffer >  frame_buffer;
};

ORB_NAMESPACE_END
