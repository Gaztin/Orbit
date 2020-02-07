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
#include "Orbit/Core/Utility/Span.h"
#include "Orbit/Graphics/Animation/Joint.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Model/Mesh.h"
#include "Orbit/Graphics/Renderer/RenderCommand.h"
#include "Orbit/Graphics/Shader/VertexLayout.h"

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS Model
{
	ORB_DISABLE_COPY( Model );

public:

	explicit Model( ByteSpan data, const VertexLayout& layout );

public:

	bool         HasJoints   ( void ) const { return m_root_node != nullptr; }
	const Joint& GetRootJoint( void ) const { return *m_root_node; }

public:

	auto begin( void ) const { return m_meshes.begin(); }
	auto end  ( void ) const { return m_meshes.end(); }

private:

	bool ParseCollada( ByteSpan data, const VertexLayout& layout );
	bool ParseOBJ    ( ByteSpan data, const VertexLayout& layout );

private:

	std::vector< Mesh > m_meshes;

	std::unique_ptr< Joint > m_root_node;

};

ORB_NAMESPACE_END
