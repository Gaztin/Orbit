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
#include "Framework/RenderQuad.h"
#include "PostFXShader.h"
#include "SceneShader.h"

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/Time/Clock.h>
#include <Orbit/Graphics/Animation/Animation.h>
#include <Orbit/Graphics/Buffer/FrameBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Geometry/Model.h>
#include <Orbit/Graphics/Renderer/DefaultRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: scene_shader_  ( scene_shader_source_.Generate(), scene_shader_source_.GetVertexLayout() )
		, post_fx_shader_( post_fx_shader_source_.Generate(), post_fx_shader_source_.GetVertexLayout() )
		, model_         ( Orbit::Asset( "models/bunny.obj" ), scene_shader_source_.GetVertexLayout() )
	{
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
		model_matrix_.Rotate( Orbit::Vector3( 0.0f, Orbit::Pi * 1.0f, 0.0f ) );
		camera_.position = Orbit::Vector3( 0.03f, 0.17f, -0.2f );
		camera_.rotation = Orbit::Vector3( 0.11f * Orbit::Pi, 0.0f, 0.0f );
	}

public:

	void OnFrame( void ) override
	{
		const float life_time  = Orbit::Clock::GetLife();
		const float delta_time = Orbit::Clock::GetDelta();

		// Clear context and framebuffer
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );
		frame_buffer_.Clear();

		// Update camera
		camera_.Update( delta_time );

		// Update scene shader uniforms
		scene_shader_.SetVertexUniform( scene_shader_source_.u_view_projection, camera_.GetViewProjection() );
		scene_shader_.SetVertexUniform( scene_shader_source_.u_model,           model_matrix_ );

		// Update post-fx shader uniforms
		post_fx_shader_.SetPixelUniform( post_fx_shader_source_.u_time, life_time );

		// Push meshes to render queue
		for( const Orbit::Mesh& mesh : model_ )
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = mesh.GetVertexBuffer();
			command.index_buffer  = mesh.GetIndexBuffer();
			command.shader        = scene_shader_;
			command.frame_buffer  = frame_buffer_;
			Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );
		}

		// Push render quad to render queue
		Orbit::RenderCommand command;
		command.vertex_buffer = render_quad_.vertex_buffer_;
		command.index_buffer  = render_quad_.index_buffer_;
		command.shader        = post_fx_shader_;
		command.textures.emplace_back( frame_buffer_.GetTexture2D() );
		Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );

		// Render scene
		Orbit::DefaultRenderer::GetInstance().Render();

		// Swap buffers
		render_context_.SwapBuffers();
	}

private:

	Orbit::RenderContext  render_context_;
	SceneShader           scene_shader_source_;
	PostFXShader          post_fx_shader_source_;
	Orbit::Shader         scene_shader_;
	Orbit::Shader         post_fx_shader_;
	Orbit::Model          model_;
	Orbit::FrameBuffer    frame_buffer_;
	Orbit::Matrix4        model_matrix_;
	Camera                camera_;
	RenderQuad            render_quad_;

};
