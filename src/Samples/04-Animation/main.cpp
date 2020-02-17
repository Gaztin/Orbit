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

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Animation/Animation.h>
#include <Orbit/Graphics/Buffer/ConstantBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Model/Model.h>
#include <Orbit/Graphics/Renderer/BasicRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Math/Vector3.h>

#include "Framework/Camera.h"
#include "AnimationShader.h"

#include <cmath>

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

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: window_         ( 800, 600 )
		, shader_         ( animation_shader )
		, model_          ( Orbit::Asset( "models/mannequin.dae" ), animation_shader.GetVertexLayout() )
		, animation_      ( Orbit::Asset( "animations/jump.dae" ) )
		, constant_buffer_( sizeof( ConstantData ) )
		, life_time_      ( 0.0f )
	{
		window_.SetTitle( "Orbit Sample (03-Model)" );
		window_.Show();
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
		model_matrix_.Translate( Orbit::Vector3( 0.0f, -2.0f, 0.0f ) );
		model_matrix_.Rotate( Orbit::Vector3( 0.0f, Orbit::Pi * 1.0f, 0.0f ) );
		camera_.position  = Orbit::Vector3( 0.0f, 540.0f, 600.0f );
		camera_.rotation  = Orbit::Vector3( 0.1f * Orbit::Pi, 1.0f * Orbit::Pi, 0.0f );
		camera_.near_clip = 256.0f;
		camera_.far_clip  = 1024.0f;
	}

public:

	void UpdateJointTransformsRecursive( const Orbit::Joint& joint, const Orbit::Matrix4& parent_pose )
	{
		const float          animation_time = std::fmod( life_time_, animation_.GetDuration() );
		const Orbit::Matrix4 local_pose     = animation_.JointPoseAtTime( joint.name, animation_time );
		const Orbit::Matrix4 pose           = ( parent_pose * local_pose );

		if( joint.id >= 0 )
			constant_data.joint_transforms[ joint.id ] = ( pose * joint.inverse_bind_transform ).Transposed();

		for( const Orbit::Joint& child : joint.children )
			UpdateJointTransformsRecursive( child, pose );
	}

	void OnFrame( float delta_time ) override
	{
		life_time_ += delta_time;

		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		camera_.Update( delta_time );

		constant_data.view_projection = camera_.GetViewProjection();

		if( model_.HasJoints() )
		{
			const Orbit::Joint& root_joint = model_.GetRootJoint();

			UpdateJointTransformsRecursive( root_joint, Orbit::Matrix4() );
		}

		constant_buffer_.Update( &constant_data, sizeof( ConstantData ) );

		for( const Orbit::Mesh& mesh : model_ )
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = mesh.vertex_buffer.get();
			command.index_buffer  = mesh.index_buffer.get();
			command.shader        = &shader_;
			command.constant_buffers[ Orbit::ShaderType::Vertex ].push_back( &constant_buffer_ );

			renderer_.QueueCommand( command );
		}

		renderer_.Render();

		render_context_.SwapBuffers();
	}

	bool IsRunning() override { return window_.IsOpen(); }

private:

	Orbit::Window         window_;
	Orbit::RenderContext  render_context_;
	Orbit::Shader         shader_;
	Orbit::Model          model_;
	Orbit::Animation      animation_;
	Orbit::ConstantBuffer constant_buffer_;
	Orbit::BasicRenderer  renderer_;
	Orbit::Matrix4        model_matrix_;

	Camera camera_;

	float life_time_;

};
