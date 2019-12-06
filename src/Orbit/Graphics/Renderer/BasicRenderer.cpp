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
#include "Orbit/Graphics/Device/RenderContext.h"
#include "Orbit/Graphics/Shader/Shader.h"

ORB_NAMESPACE_BEGIN

void BasicRenderer::QueueCommand( const RenderCommand& command )
{
	m_commands.push_back( command );
}

void BasicRenderer::Render( void )
{
	Private::RenderContextData& context_data = RenderContext::GetInstance().GetPrivateData();

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

		switch( context_data.index() )
		{
			default: break;

		#if( ORB_HAS_D3D11 )

			case( unique_index_v< Private::_RenderContextDataD3D11, Private::RenderContextData > ):
			{
				Private::_RenderContextDataD3D11& d3d11 = std::get< Private::_RenderContextDataD3D11 >( context_data );

				d3d11.device_context->DrawIndexed( static_cast< UINT >( command.index_buffer->GetCount() ), 0, 0 );

				break;
			}

		#endif
		#if( ORB_HAS_OPENGL )

			case( unique_index_v< Private::_RenderContextDataOpenGL, Private::RenderContextData > ):
			{
				Private::_RenderContextDataOpenGL& gl = std::get< Private::_RenderContextDataOpenGL >( context_data );

				OpenGL::IndexType index_type { };

				switch( command.index_buffer->GetFormat() )
				{
					case IndexFormat::Byte:       { index_type = OpenGL::IndexType::Byte;  } break;
					case IndexFormat::Word:       { index_type = OpenGL::IndexType::Short; } break;
					case IndexFormat::DoubleWord: { index_type = OpenGL::IndexType::Int;   } break;
				}

				gl.functions->draw_elements( OpenGL::DrawMode::Triangles, static_cast< GLsizei >( command.index_buffer->GetCount() ), index_type, nullptr );

				break;
			}

		#endif

		}

//		command.index_buffer->Unbind();
		command.shader->Unbind();
//		command.vertex_buffer->Unbind();
	}

	m_commands.clear();
}

ORB_NAMESPACE_END
