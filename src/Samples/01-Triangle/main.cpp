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
#include <Orbit/Graphics/Buffer/IndexBuffer.h>
#include <Orbit/Graphics/Buffer/Texture2D.h>
#include <Orbit/Graphics/Buffer/VertexBuffer.h>
#include <Orbit/Graphics/Device/RenderContext.h>
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

const Orbit::VertexLayout vertex_layout
{
	{ "POSITION", Orbit::VertexComponent::Vec4 },
	{ "COLOR",    Orbit::VertexComponent::Vec4 },
	{ "TEXCOORD", Orbit::VertexComponent::Vec2 },
};

constexpr std::string_view shader_source = R"(
#if defined( GLSL )

#  if defined( VERTEX )

ORB_ATTRIBUTE vec4 a_position;
ORB_ATTRIBUTE vec4 a_color;
ORB_ATTRIBUTE vec2 a_texcoord;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;

void main()
{
	v_position = a_position;
	v_color    = a_color;
	v_texcoord = a_texcoord;

	gl_Position = v_position;
}

#  elif defined( FRAGMENT )

uniform sampler2D diffuse_texture;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;

void main()
{
	vec4 tex_color = texture( diffuse_texture, v_texcoord );
	vec4 out_color = tex_color * v_color;

	ORB_SET_OUT_COLOR( out_color );
}

#  endif

#elif defined( HLSL )

Texture2D diffuse_texture;

SamplerState texture_sampler;

struct VertexData
{
	float4 position : POSITION;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
};

struct PixelData
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
};

PixelData VSMain( VertexData input )
{
	PixelData output;
	output.position = input.position;
	output.color    = input.color;
	output.texcoord = input.texcoord;

	return output;
}

float4 PSMain( PixelData input ) : SV_TARGET
{
	float4 tex_color = diffuse_texture.Sample( texture_sampler, input.texcoord );
	float4 out_color = tex_color * input.color;

	return out_color;
}

#endif
)";

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

ORB_APP_DECL( SampleApp )
{
public:

	SampleApp( void )
		: m_window( 800, 600 )
		, m_shader( shader_source, vertex_layout )
		, m_vertex_buffer( vertex_data )
		, m_index_buffer( index_data )
		, m_texture( 4, 4, texture_data )
		, m_time( 0.0f )
	{
		m_window.SetTitle( "Orbit Sample (01-Triangle)" );
		m_window.Show();
		m_render_context.SetClearColor( 0.0f, 0.0f, 0.5f );
	}

	void OnFrame( float /*delta_time*/ ) override
	{
		m_window.PollEvents();
		m_render_context.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		Orbit::RenderCommand command;
		command.vertex_buffer = &m_vertex_buffer;
		command.index_buffer  = &m_index_buffer;
		command.shader        = &m_shader;
		command.textures.push_back( &m_texture );
		m_renderer.QueueCommand( command );
		m_renderer.Render();

		m_render_context.SwapBuffers();
	}

	bool IsRunning( void ) override { return m_window.IsOpen(); }

private:

	Orbit::Window           m_window;
	Orbit::RenderContext    m_render_context;
	Orbit::Shader           m_shader;
	Orbit::VertexBuffer     m_vertex_buffer;
	Orbit::IndexBuffer      m_index_buffer;
	Orbit::Texture2D        m_texture;
	Orbit::BasicRenderer    m_renderer;
	float                   m_time;

};
