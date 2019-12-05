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

#include "BasicRenderer.h"

#include "Orbit/Graphics/Buffer/ConstantBuffer.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/Texture2D.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Shader/Shader.h"

ORB_NAMESPACE_BEGIN

void BasicRenderer::QueueCommand( const RenderCommand& command )
{
	m_commands.push_back( command );
}

void BasicRenderer::Render( void )
{
	for( RenderCommand& command : m_commands )
	{
		for( size_t i = 0; i < command.textures.size(); ++i )
		{
			command.textures[ i ]->Bind( static_cast< uint32_t >( i ) );
		}

		command.vertex_buffer->Bind();
		command.shader->Bind();
		command.index_buffer->Bind();

		for( size_t i = 0; i < command.constant_buffers.size(); ++i )
		{
			command.constant_buffers[ i ]->Bind( ShaderType::Vertex, static_cast< uint32_t >( i ) );
		}

		command.shader->Draw( *command.index_buffer );

//		command.index_buffer->Unbind();
		command.shader->Unbind();
//		command.vertex_buffer->Unbind();
	}

	m_commands.clear();
}

ORB_NAMESPACE_END
