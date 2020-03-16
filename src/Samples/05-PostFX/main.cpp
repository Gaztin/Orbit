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
#include <Orbit/Graphics/Buffer/FrameBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Geometry/Model.h>
#include <Orbit/Graphics/Renderer/DefaultRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Math/Vector3.h>

#include "Framework/Camera.h"
#include "Framework/RenderQuad.h"
#include "PostFXShader.h"
#include "SceneShader.h"

#include <cmath>

static SceneShader  scene_shader;
static PostFXShader post_fx_shader;

struct SceneVertexConstantData
{
	Orbit::Matrix4 view_projection;
	Orbit::Matrix4 model;

} scene_vertex_constant_data;

struct PostFXPixelConstantData
{
	float time;

} post_fx_pixel_constant_data;

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: window_                       ( 800, 600 )
		, scene_shader_                 ( scene_shader.Generate(), scene_shader.GetVertexLayout() )
		, post_fx_shader_               ( post_fx_shader.Generate(), post_fx_shader.GetVertexLayout() )
		, model_                        ( Orbit::Asset( "models/bunny.obj" ), scene_shader.GetVertexLayout() )
		, scene_vertex_constant_buffer_ ( sizeof( SceneVertexConstantData ) )
		, post_fx_pixel_constant_buffer_( sizeof( PostFXPixelConstantData ) )
	{
		window_.SetTitle( "Orbit Sample (05-PostFX)" );
		window_.Show();
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
		model_matrix_.Rotate( Orbit::Vector3( 0.0f, Orbit::Pi * 1.0f, 0.0f ) );
		camera_.position = Orbit::Vector3( 0.03f, 0.17f, -0.2f );
		camera_.rotation = Orbit::Vector3( 0.11f * Orbit::Pi, 0.0f, 0.0f );
	}

public:

	void OnFrame( float delta_time ) override
	{
		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );
		frame_buffer_.Clear();

		camera_.Update( delta_time );

		scene_vertex_constant_data.view_projection = camera_.GetViewProjection();
		scene_vertex_constant_data.model           = model_matrix_;
		scene_vertex_constant_buffer_.Update( &scene_vertex_constant_data, sizeof( SceneVertexConstantData ) );

		post_fx_pixel_constant_data.time += delta_time;
		post_fx_pixel_constant_buffer_.Update( &post_fx_pixel_constant_data, sizeof( PostFXPixelConstantData ) );

		for( const Orbit::Mesh& mesh : model_ )
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = mesh.GetVertexBuffer();
			command.index_buffer  = mesh.GetIndexBuffer();
			command.shader        = scene_shader_;
			command.frame_buffer  = frame_buffer_;
			command.constant_buffers[ Orbit::ShaderType::Vertex ].emplace_back( scene_vertex_constant_buffer_ );

			Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );
		}

		/* Render quad */
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = render_quad_.vertex_buffer_;
			command.index_buffer  = render_quad_.index_buffer_;
			command.shader        = post_fx_shader_;
			command.textures.emplace_back( frame_buffer_.GetTexture2D() );
			command.constant_buffers[ Orbit::ShaderType::Fragment ].emplace_back( post_fx_pixel_constant_buffer_ );

			Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );
		}

		Orbit::DefaultRenderer::GetInstance().Render();

		render_context_.SwapBuffers();
	}

	bool IsRunning( void ) override { return window_.IsOpen(); }

private:

	Orbit::Window         window_;
	Orbit::RenderContext  render_context_;
	Orbit::Shader         scene_shader_;
	Orbit::Shader         post_fx_shader_;
	Orbit::Model          model_;
	Orbit::ConstantBuffer scene_vertex_constant_buffer_;
	Orbit::ConstantBuffer post_fx_pixel_constant_buffer_;
	Orbit::FrameBuffer    frame_buffer_;
	Orbit::Matrix4        model_matrix_;

	Camera     camera_;
	RenderQuad render_quad_;

};
