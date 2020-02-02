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
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Model/Model.h>
#include <Orbit/Graphics/Renderer/BasicRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Math/Vector3.h>

#include "Framework/Camera.h"
#include "AnimationShader.h"

static AnimationShader animation_shader;

std::tuple constant_data = std::make_tuple( Orbit::Matrix4(), Orbit::Matrix4(), Orbit::Matrix4() );

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
		, m_shader( animation_shader )
		, m_model( Orbit::Asset( "models/mannequin.dae" ), animation_shader.GetVertexLayout() )
		, m_constant_buffer( constant_data )
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
			auto& [ view_projection, model, model_inverse ] = constant_data;

			/* Calculate model-view-projection matrix */
			{
				m_model_matrix.Rotate( Orbit::Vector3( 0.0f, Orbit::Pi * 0.5f * delta_time, 0.0f ) );

				view_projection = m_camera.GetViewProjection();
				model           = m_model_matrix;
				model_inverse   = m_model_matrix;
				model_inverse.Invert();
			}

			m_constant_buffer.Update( constant_data );
		}

		m_window.PollEvents();
		m_render_context.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		m_camera.Update( delta_time );

		Orbit::RenderCommand command = m_model.MakeRenderCommand();
		command.constant_buffers[ Orbit::ShaderType::Vertex ].push_back( &m_constant_buffer );
		command.shader = &m_shader;

		m_renderer.QueueCommand( command );
		m_renderer.Render();

		m_render_context.SwapBuffers();
	}

	bool IsRunning() override { return m_window.IsOpen(); }

private:

	Orbit::Window         m_window;
	Orbit::RenderContext  m_render_context;
	Orbit::Shader         m_shader;
	Orbit::Model          m_model;
	Orbit::ConstantBuffer m_constant_buffer;
	Orbit::BasicRenderer  m_renderer;
	Orbit::Matrix4        m_model_matrix;

	Camera m_camera;

};
