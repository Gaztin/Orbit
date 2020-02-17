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
#include "ModelShader.h"

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

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: window_                  ( 800, 600 )
		, shader_                  ( model_shader )
		, model_                   ( Orbit::Asset( "models/teapot.obj" ), model_shader.GetVertexLayout() )
		, vertex_constant_buffer_  ( sizeof( VertexConstantData ) )
		, fragment_constant_buffer_( sizeof( FragmentConstantData ) )
		, texture_                 ( 4, 4, texture_data )
	{
		window_.SetTitle( "Orbit Sample (03-Model)" );
		window_.Show();
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
		model_matrix_.Translate( Orbit::Vector3( 0.0f, -2.0f, 0.0f ) );
	}

public:

	void OnFrame( float delta_time ) override
	{
		/* Update constant buffers */
		{
			model_matrix_.Rotate( Orbit::Vector3( 0.0f, 0.5f * Orbit::Pi * delta_time, 0.0f ) );

			vertex_constant_data.view_projection = camera_.GetViewProjection();
			vertex_constant_data.model           = model_matrix_;
			vertex_constant_data.model_inverse   = model_matrix_;
			vertex_constant_data.model_inverse.Invert();

			/* Light position */
			fragment_constant_data.light_dir = Orbit::Vector3( 1.0f, -1.0f, 1.0f );

			vertex_constant_buffer_.Update( &vertex_constant_data, sizeof( VertexConstantData ) );
			fragment_constant_buffer_.Update( &fragment_constant_data, sizeof( FragmentConstantData ) );
		}

		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		camera_.Update( delta_time );

		for( const Orbit::Mesh& mesh : model_ )
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = mesh.vertex_buffer.get();
			command.index_buffer  = mesh.index_buffer.get();
			command.shader        = &shader_;
			command.constant_buffers[ Orbit::ShaderType::Vertex   ].push_back( &vertex_constant_buffer_ );
			command.constant_buffers[ Orbit::ShaderType::Fragment ].push_back( &fragment_constant_buffer_ );
			command.textures.push_back( &texture_ );

			renderer_.QueueCommand( command );
		}

		renderer_.Render();
		render_context_.SwapBuffers();
	}

	bool IsRunning() override { return window_.IsOpen(); }

private:

	Orbit::Window            window_;
	Orbit::RenderContext     render_context_;
	Orbit::Shader            shader_;
	Orbit::Model             model_;
	Orbit::ConstantBuffer    vertex_constant_buffer_;
	Orbit::ConstantBuffer    fragment_constant_buffer_;
	Orbit::Texture2D         texture_;
	Orbit::BasicRenderer     renderer_;
	Orbit::Matrix4           model_matrix_;

	Camera camera_;

};
