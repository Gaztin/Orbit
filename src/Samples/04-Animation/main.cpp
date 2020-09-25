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

#include "Framework/Camera.h"
#include "AnimationShader.h"

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/Time/Clock.h>
#include <Orbit/Graphics/Animation/Animation.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Geometry/Mesh.h>
#include <Orbit/Graphics/ModelFormats/COLLADAFile.h>
#include <Orbit/Graphics/Renderer/DefaultRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: shader_    ( shader_source_.Generate(), shader_source_.GetVertexLayout() )
		, model_data_( Orbit::COLLADAFile( Orbit::Asset( "models/mannequin.dae" ), shader_source_.GetVertexLayout() ).GetModelData() )
		, animation_ ( Orbit::COLLADAFile( Orbit::Asset( "animations/jump.dae" ), shader_source_.GetVertexLayout() ).GetAnimationData().keyframes )
	{
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
		const float          life_time      = Orbit::Clock::GetLife();
		const float          animation_time = std::fmod( life_time, static_cast< float >( animation_.GetDuration() ) );
		const Orbit::Matrix4 local_pose     = animation_.JointPoseAtTime( joint.name, animation_time );
		const Orbit::Matrix4 pose           = ( parent_pose * local_pose );

		if( joint.id >= 0 )
			joint_transforms_[ joint.id ] = ( pose * joint.inverse_bind_transform ).Transposed();

		for( const Orbit::Joint& child : joint.children )
			UpdateJointTransformsRecursive( child, pose );
	}

	void OnFrame( void ) override
	{
		const float delta_time = Orbit::Clock::GetDelta();

		// Clear context
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		// Update camera
		camera_.Update( delta_time );

		// Update joint transforms
		UpdateJointTransformsRecursive( model_data_.root_joint, Orbit::Matrix4() );

		// Update uniforms
		shader_.SetVertexUniform( shader_source_.u_view_projection, camera_.GetViewProjection() );
		shader_.SetVertexUniform( shader_source_.u_joint_transforms, joint_transforms_ );

		// Push meshes to render queue
		for( auto& mesh : model_data_.meshes )
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = mesh->GetVertexBuffer();
			command.index_buffer  = mesh->GetIndexBuffer();
			command.shader        = shader_;
			Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );
		}

		// Render scene
		Orbit::DefaultRenderer::GetInstance().Render();

		// Swap buffers
		render_context_.SwapBuffers();
	}

private:

	using JointTransformArray = std::array< Orbit::Matrix4, AnimationShader::joint_transform_count >;

private:

	Orbit::RenderContext          render_context_;
	AnimationShader               shader_source_;
	Orbit::Shader                 shader_;
	Orbit::COLLADAFile::ModelData model_data_;
	Orbit::Animation              animation_;
	Orbit::Matrix4                model_matrix_;
	Camera                        camera_;
	JointTransformArray           joint_transforms_;

};
