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

struct ConstantData
{
	Orbit::Matrix4 view_projection;

	std::array< Orbit::Matrix4, AnimationShader::joint_transform_count > joint_transforms;

} constant_data;

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
		, m_constant_buffer( sizeof( ConstantData ) )
		, m_life_time( 0.0f )
	{
		m_window.SetTitle( "Orbit Sample (03-Model)" );
		m_window.Show();
		m_render_context.SetClearColor( 0.0f, 0.0f, 0.5f );
		m_model_matrix.Translate( Orbit::Vector3( 0.0f, -2.0f, 0.0f ) );
		m_model_matrix.Rotate( Orbit::Vector3( 0.0f, Orbit::Pi * 1.0f, 0.0f ) );
		m_camera.position = Orbit::Vector3( 0.0f, 3.0f, -6.0f );
		m_camera.rotation = Orbit::Vector3( Orbit::Pi * 0.1f, 0.0f, 0.0f );
	}

public:

	void UpdateJointTransformsRecursive( const Orbit::Joint& joint, Orbit::Matrix4 world_transform )
	{
		for( const Orbit::Joint& child : joint.children )
		{
			const Orbit::Matrix4 child_world_transform = world_transform * child.local_bind_transform;

			constant_data.joint_transforms[ child.id ] = child_world_transform;

			UpdateJointTransformsRecursive( child, child_world_transform );
		}
	}

	void OnFrame( float delta_time ) override
	{
		m_life_time += delta_time;

		m_window.PollEvents();
		m_render_context.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		m_camera.Update( delta_time );

		constant_data.view_projection = m_camera.GetViewProjection();

		const Orbit::Joint& root_joint = m_model.GetRootJoint();

		UpdateJointTransformsRecursive( root_joint, Orbit::Matrix4() );

		m_constant_buffer.Update( &constant_data, sizeof( ConstantData ) );

		for( const Orbit::Mesh& mesh : m_model )
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = mesh.vertex_buffer.get();
			command.index_buffer  = mesh.index_buffer.get();
			command.shader        = &m_shader;
			command.constant_buffers[ Orbit::ShaderType::Vertex ].push_back( &m_constant_buffer );

			m_renderer.QueueCommand( command );
		}

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

	float m_life_time;

};
