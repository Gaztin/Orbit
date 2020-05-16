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

#include "BasicRenderer.h"

#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Buffer/ConstantBuffer.h"
#include "Orbit/Graphics/Buffer/FrameBuffer.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Context/RenderContext.h"
#include "Orbit/Graphics/Shader/Shader.h"
#include "Orbit/Graphics/Texture/Texture2D.h"

ORB_NAMESPACE_BEGIN

void BasicRenderer::QueueCommand( const RenderCommand& command )
{
	commands_.push_back( command );
}

void BasicRenderer::Render( void )
{
	for( RenderCommand& command : commands_ )
	{
		if( command.frame_buffer )
			command.frame_buffer->Bind();

		for( size_t i = 0; i < command.textures.size(); ++i )
			command.textures[ i ]->Bind( static_cast< uint32_t >( i ) );

		command.vertex_buffer->Bind();
		command.shader->Bind();

		if( command.index_buffer )
			command.index_buffer->Bind();

		BindConstantBuffers( command );
		APIDraw( command );
		UnbindConstantBuffers( command );

//		if( command.index_buffer )
//			command.index_buffer->Unbind();

		command.shader->Unbind();
//		command.vertex_buffer->Unbind();

		for( size_t i = 0; i < command.textures.size(); ++i )
			command.textures[ i ]->Unbind( static_cast< uint32_t >( i ) );

		if( command.frame_buffer )
			command.frame_buffer->Unbind();
	}

	commands_.clear();
}

ORB_NAMESPACE_END
