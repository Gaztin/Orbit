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
#include "CubeShader.h"

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/Input/Input.h>
#include <Orbit/Core/Input/Key.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/IO/Log.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Buffer/ConstantBuffer.h>
#include <Orbit/Graphics/Buffer/IndexBuffer.h>
#include <Orbit/Graphics/Buffer/Texture2D.h>
#include <Orbit/Graphics/Buffer/VertexBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Renderer/BasicRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Math/Literals.h>
#include <Orbit/Math/Matrix4.h>
#include <Orbit/Math/Vector2.h>
#include <Orbit/Math/Vector3.h>
#include <Orbit/Math/Vector4.h>

struct Vertex
{
	Orbit::Vector4 pos;
	Orbit::Color   color;
	Orbit::Vector2 texcoord;
};

static CubeShader cube_shader;

const std::initializer_list< Vertex > vertex_data
{
	/* Front face */
	{ Orbit::Vector4( -1.f, -1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 1.f ) },
	{ Orbit::Vector4( -1.f,  1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 0.f ) },
	{ Orbit::Vector4(  1.f,  1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 0.f ) },
	{ Orbit::Vector4(  1.f, -1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 1.f ) },
	/* Right face */
	{ Orbit::Vector4( 1.f, -1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 1.f ) },
	{ Orbit::Vector4( 1.f,  1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 0.f ) },
	{ Orbit::Vector4( 1.f,  1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 0.f ) },
	{ Orbit::Vector4( 1.f, -1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 1.f ) },
	/* Back face */
	{ Orbit::Vector4(  1.f, -1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 1.f ) },
	{ Orbit::Vector4(  1.f,  1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 0.f ) },
	{ Orbit::Vector4( -1.f,  1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 0.f ) },
	{ Orbit::Vector4( -1.f, -1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 1.f ) },
	/* Left face */
	{ Orbit::Vector4( -1.f, -1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 1.f ) },
	{ Orbit::Vector4( -1.f,  1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 0.f ) },
	{ Orbit::Vector4( -1.f,  1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 0.f ) },
	{ Orbit::Vector4( -1.f, -1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 1.f ) },
	/* Bottom face */
	{ Orbit::Vector4( -1.f, -1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 1.f ) },
	{ Orbit::Vector4(  1.f, -1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 0.f ) },
	{ Orbit::Vector4(  1.f, -1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 0.f ) },
	{ Orbit::Vector4( -1.f, -1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 1.f ) },
	/* Top face */
	{ Orbit::Vector4(  1.f,  1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 1.f ) },
	{ Orbit::Vector4(  1.f,  1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 0.f, 0.f ) },
	{ Orbit::Vector4( -1.f,  1.f, -1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 0.f ) },
	{ Orbit::Vector4( -1.f,  1.f,  1.f, 1.f ), Orbit::Color( 1.f, 1.f, 1.f ), Orbit::Vector2( 1.f, 1.f ) },
};

const std::initializer_list< uint16_t > index_data
{
	/* Front face */
	0, 1, 2,
	0, 2, 3,
	/* Right face */
	4, 5, 6,
	4, 6, 7,
	/* Back face */
	8, 9, 10,
	8, 10, 11,
	/* Left face */
	12, 13, 14,
	12, 14, 15,
	/* Top face */
	16, 17, 18,
	16, 18, 19,
	/* Bottom face */
	20, 21, 22,
	20, 22, 23,
};

struct ConstantData
{
	Orbit::Matrix4 mvp;

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
		, shader_         ( cube_shader )
		, vertex_buffer_  ( vertex_data )
		, index_buffer_   ( index_data )
		, constant_buffer_( sizeof( ConstantData ) )
		, texture_        ( 4, 4, texture_data )
	{
		window_.SetTitle( "Orbit Sample (02-Cube)" );
		window_.Show();
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
	}

public:

	void OnFrame( float delta_time ) override
	{
		/* Update constant buffer */
		{
			model_.Rotate( Orbit::Vector3( 0.0f, 0.5f * Orbit::Pi * delta_time, 0.0f ) );

			constant_data.mvp = model_ * camera_.GetViewProjection();

			constant_buffer_.Update( &constant_data, sizeof( ConstantData ) );
		}

		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		camera_.Update( delta_time );

		Orbit::RenderCommand command;
		command.vertex_buffer = &vertex_buffer_;
		command.index_buffer  = &index_buffer_;
		command.shader        = &shader_;
		command.constant_buffers[ Orbit::ShaderType::Vertex ].push_back( &constant_buffer_ );
		command.textures.push_back( &texture_ );

		renderer_.QueueCommand( command );
		renderer_.Render();

		render_context_.SwapBuffers();
	}

	bool IsRunning( void ) override { return window_.IsOpen(); }

private:

	Orbit::Window            window_;
	Orbit::RenderContext     render_context_;
	Orbit::Shader            shader_;
	Orbit::VertexBuffer      vertex_buffer_;
	Orbit::IndexBuffer       index_buffer_;
	Orbit::ConstantBuffer    constant_buffer_;
	Orbit::Texture2D         texture_;
	Orbit::BasicRenderer     renderer_;
	Orbit::Matrix4           model_;

	Camera camera_;

};
