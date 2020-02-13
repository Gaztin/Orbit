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

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Buffer/ConstantBuffer.h>
#include <Orbit/Graphics/Buffer/IndexBuffer.h>
#include <Orbit/Graphics/Buffer/Texture2D.h>
#include <Orbit/Graphics/Buffer/VertexBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Model/Model.h>
#include <Orbit/Graphics/Renderer/BasicRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Math/Literals.h>
#include <Orbit/Math/Matrix4.h>
#include <Orbit/Math/Vector2.h>
#include <Orbit/Math/Vector3.h>
#include <Orbit/Math/Vector4.h>

#include "Framework/Camera.h"
#include "ModelShader.h"

static ModelShader model_shader;

struct VertexConstantData
{
	Orbit::Matrix4 view_projection;
	Orbit::Matrix4 model;
	Orbit::Matrix4 model_inverse;

} vertex_constant_data;

struct FragmentConstantData
{
	Orbit::Vector3 light_dir;

} fragment_constant_data;

const uint32_t texture_data[]
{
	0xffff00ff, 0xffff00ff, 0xff00ff00, 0xff00ff00,
	0xffff00ff, 0xffff00ff, 0xff00ff00, 0xff00ff00,
	0xff00ff00, 0xff00ff00, 0xffff00ff, 0xffff00ff,
	0xff00ff00, 0xff00ff00, 0xffff00ff, 0xffff00ff,
};

ORB_APP_DECL( SampleApp )
{
public:

	SampleApp( void )
		: m_window( 800, 600 )
		, m_shader( model_shader )
		, m_model( Orbit::Asset( "models/teapot.obj" ), model_shader.GetVertexLayout() )
		, m_vertex_constant_buffer( sizeof( VertexConstantData ) )
		, m_fragment_constant_buffer( sizeof( FragmentConstantData ) )
		, m_texture( 4, 4, texture_data )
	{
		m_window.SetTitle( "Orbit Sample (03-Model)" );
		m_window.Show();
		m_render_context.SetClearColor( 0.0f, 0.0f, 0.5f );
		m_model_matrix.Translate( Orbit::Vector3( 0.0f, -2.0f, 0.0f ) );
	}

public:

	void OnFrame( float delta_time ) override
	{
		/* Update constant buffers */
		{
			m_model_matrix.Rotate( Orbit::Vector3( 0.0f, 0.5f * Orbit::Pi * delta_time, 0.0f ) );

			vertex_constant_data.view_projection = m_camera.GetViewProjection();
			vertex_constant_data.model           = m_model_matrix;
			vertex_constant_data.model_inverse   = m_model_matrix;
			vertex_constant_data.model_inverse.Invert();

			/* Light position */
			fragment_constant_data.light_dir = Orbit::Vector3( 1.0f, -1.0f, 1.0f );

			m_vertex_constant_buffer.Update( &vertex_constant_data, sizeof( VertexConstantData ) );
			m_fragment_constant_buffer.Update( &fragment_constant_data, sizeof( FragmentConstantData ) );
		}

		m_window.PollEvents();
		m_render_context.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		m_camera.Update( delta_time );

		for( const Orbit::Mesh& mesh : m_model )
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = mesh.vertex_buffer.get();
			command.index_buffer  = mesh.index_buffer.get();
			command.shader        = &m_shader;
			command.constant_buffers[ Orbit::ShaderType::Vertex   ].push_back( &m_vertex_constant_buffer );
			command.constant_buffers[ Orbit::ShaderType::Fragment ].push_back( &m_fragment_constant_buffer );
			command.textures.push_back( &m_texture );

			m_renderer.QueueCommand( command );
		}

		m_renderer.Render();
		m_render_context.SwapBuffers();
	}

	bool IsRunning() override { return m_window.IsOpen(); }

private:

	Orbit::Window            m_window;
	Orbit::RenderContext     m_render_context;
	Orbit::Shader            m_shader;
	Orbit::Model             m_model;
	Orbit::ConstantBuffer    m_vertex_constant_buffer;
	Orbit::ConstantBuffer    m_fragment_constant_buffer;
	Orbit::Texture2D         m_texture;
	Orbit::BasicRenderer     m_renderer;
	Orbit::Matrix4           m_model_matrix;

	Camera m_camera;

};
