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
#include "SceneShader.h"

#include <cmath>

static SceneShader scene_shader;

struct ConstantData
{
	Orbit::Matrix4 view_projection;

} constant_data;

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: window_         ( 800, 600 )
		, shader_         ( scene_shader )
		, model_          ( Orbit::Asset( "models/teapot.obj" ), scene_shader.GetVertexLayout() )
		, constant_buffer_( sizeof( ConstantData ) )
	{
		window_.SetTitle( "Orbit Sample (03-Model)" );
		window_.Show();
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
		model_matrix_.Translate( Orbit::Vector3( 0.0f, -2.0f, 0.0f ) );
		model_matrix_.Rotate( Orbit::Vector3( 0.0f, Orbit::Pi * 1.0f, 0.0f ) );
		camera_.position = Orbit::Vector3( 0.0f, 3.0f, -5.0f );
		camera_.rotation = Orbit::Vector3( 0.1f * Orbit::Pi, 0.0f, 0.0f );
	}

public:

	void OnFrame( float delta_time ) override
	{
		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		camera_.Update( delta_time );

		constant_data.view_projection = camera_.GetViewProjection();
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

	bool IsRunning( void ) override { return window_.IsOpen(); }

private:

	Orbit::Window         window_;
	Orbit::RenderContext  render_context_;
	Orbit::Shader         shader_;
	Orbit::Model          model_;
	Orbit::ConstantBuffer constant_buffer_;
	Orbit::BasicRenderer  renderer_;
	Orbit::Matrix4        model_matrix_;

	Camera camera_;

};
