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

#include "TriangleShader.h"

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Buffer/IndexBuffer.h>
#include <Orbit/Graphics/Buffer/Texture2D.h>
#include <Orbit/Graphics/Buffer/VertexBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Renderer/BasicRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Math/Vector2.h>
#include <Orbit/Math/Vector4.h>

struct Vertex
{
	Orbit::Vector4 pos;
	Orbit::Color   color;
	Orbit::Vector2 texcoord;
};

static TriangleShader triangle_shader;

const std::initializer_list< Vertex > vertex_data
{
	{ Orbit::Vector4( -1.0f / Orbit::PythagorasConstant, -1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f ), Orbit::Color( 1.0f, 0.0f, 1.0f, 1.0f ), Orbit::Vector2( 0.0f, 0.0f ) },
	{ Orbit::Vector4(  0.0f,                              1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f ), Orbit::Color( 0.0f, 1.0f, 1.0f, 1.0f ), Orbit::Vector2( 0.5f, 1.0f ) },
	{ Orbit::Vector4(  1.0f / Orbit::PythagorasConstant, -1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f ), Orbit::Color( 1.0f, 1.0f, 0.0f, 1.0f ), Orbit::Vector2( 1.0f, 0.0f ) },
};

const std::initializer_list< uint16_t > index_data
{
	0, 1, 2,
};

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
		: window_       { 800, 600 }
		, shader_       { triangle_shader }
		, vertex_buffer_{ vertex_data }
		, index_buffer_ { index_data }
		, texture_      { 4, 4, texture_data }
		, time_         { 0.0f }
	{
		window_.SetTitle( "Orbit Sample (01-Triangle)" );
		window_.Show();
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
	}

	void OnFrame( float /*delta_time*/ ) override
	{
		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		Orbit::RenderCommand command;
		command.vertex_buffer = &vertex_buffer_;
		command.index_buffer  = &index_buffer_;
		command.shader        = &shader_;
		command.textures.push_back( &texture_ );
		renderer_.QueueCommand( command );
		renderer_.Render();

		render_context_.SwapBuffers();
	}

	bool IsRunning( void ) override { return window_.IsOpen(); }

private:

	Orbit::Window           window_;
	Orbit::RenderContext    render_context_;
	Orbit::Shader           shader_;
	Orbit::VertexBuffer     vertex_buffer_;
	Orbit::IndexBuffer      index_buffer_;
	Orbit::Texture2D        texture_;
	Orbit::BasicRenderer    renderer_;
	float                   time_;

};
