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

#include "IRenderer.h"

#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Buffer/ConstantBuffer.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Context/RenderContext.h"
#include "Orbit/Graphics/Renderer/RenderCommand.h"
#include "Orbit/Graphics/Shader/Shader.h"

ORB_NAMESPACE_BEGIN

void IRenderer::BindConstantBuffers( const RenderCommand& command )
{
	uint32_t global_slot = 0;

	for( auto& constant_buffers : command.constant_buffers )
	{
		for( size_t i = 0; i < constant_buffers.second.size(); ( ++i, ++global_slot ) )
		{
			const uint32_t local_slot     = static_cast< uint32_t >( i );
			const auto&    shader_details = command.shader->GetPrivateDetails();

			constant_buffers.second[ i ]->Bind( constant_buffers.first, local_slot, global_slot );

		#if( ORB_HAS_OPENGL )

			if( shader_details.index() == unique_index_v< Private::_ShaderDetailsOpenGL, Private::ShaderDetails > )
			{
				auto& shader_gl = std::get< Private::_ShaderDetailsOpenGL >( shader_details );

				glUniformBlockBinding( shader_gl.program, global_slot, global_slot );
			}

		#endif // ORB_HAS_OPENGL

		}
	}
}

void IRenderer::UnbindConstantBuffers( const RenderCommand& command )
{
	uint32_t global_slot = 0;

	for( auto& constant_buffers : command.constant_buffers )
	{
		for( size_t i = 0; i < constant_buffers.second.size(); ( ++i, ++global_slot ) )
		{
			const uint32_t local_slot = static_cast< uint32_t >( i );

			constant_buffers.second[ i ]->Unbind( constant_buffers.first, local_slot, global_slot );
		}
	}
}

void IRenderer::APIDraw( const RenderCommand& command )
{
	auto& context_details = RenderContext::GetInstance().GetPrivateDetails();

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& d3d11 = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			switch( command.topology )
			{
				case Topology::Points:    { d3d11.device_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST    ); } break;
				case Topology::Lines:     { d3d11.device_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST     ); } break;
				case Topology::Triangles: { d3d11.device_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ); } break;
			}

			if( command.index_buffer ) d3d11.device_context->DrawIndexed( static_cast< UINT >( command.index_buffer->GetCount() ), 0, 0 );
			else                       d3d11.device_context->Draw( command.vertex_buffer->GetCount(), 0 );

		} break;

	#endif
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			OpenGLIndexType index_type = { };
			OpenGLDrawMode  draw_mode  = { };

			switch( command.index_buffer->GetFormat() )
			{
				case IndexFormat::Byte:       { index_type = OpenGLIndexType::Byte;  } break;
				case IndexFormat::Word:       { index_type = OpenGLIndexType::Short; } break;
				case IndexFormat::DoubleWord: { index_type = OpenGLIndexType::Int;   } break;
			}

			switch( command.topology )
			{
				case Topology::Points:    { draw_mode = OpenGLDrawMode::Points;    } break;
				case Topology::Lines:     { draw_mode = OpenGLDrawMode::Lines;     } break;
				case Topology::Triangles: { draw_mode = OpenGLDrawMode::Triangles; } break;
			}

			if( command.index_buffer ) glDrawElements( draw_mode, static_cast< GLsizei >( command.index_buffer->GetCount() ), index_type, nullptr );
			else                       glDrawArrays( draw_mode, 0, command.vertex_buffer->GetCount() );

		} break;

	#endif

	}
}

ORB_NAMESPACE_END
