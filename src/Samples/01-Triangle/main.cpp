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
#include <Orbit/Graphics/Shader/FragmentShader.h>
#include <Orbit/Graphics/Shader/GraphicsPipeline.h>
#include <Orbit/Graphics/Shader/VertexShader.h>
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

const std::initializer_list< Vertex > triangle_vertices
{
	{ Orbit::Vector4( -1.0f / Orbit::PythagorasConstant, -1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f ), Orbit::Color( 1.0f, 0.0f, 1.0f, 1.0f ), Orbit::Vector2( 0.0f, 0.0f ) },
	{ Orbit::Vector4(  0.0f,                              1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f ), Orbit::Color( 0.0f, 1.0f, 1.0f, 1.0f ), Orbit::Vector2( 0.5f, 1.0f ) },
	{ Orbit::Vector4(  1.0f / Orbit::PythagorasConstant, -1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f ), Orbit::Color( 1.0f, 1.0f, 0.0f, 1.0f ), Orbit::Vector2( 1.0f, 0.0f ) },
};

const std::initializer_list< uint16_t > triangle_indices
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
		, m_render_context( m_window )
		, m_vertex_shader( Orbit::Asset( "triangle_shader.vs" ) )
		, m_fragment_shader( Orbit::Asset( "triangle_shader.fs" ) )
		, m_triangle_vertex_buffer( triangle_vertices )
		, m_triangle_index_buffer( triangle_indices )
		, m_texture_2d( 4, 4, texture_data )
		, m_time( 0.0f )
	{
		m_window.SetTitle( "Orbit Sample (01-Triangle)" );
		m_window.Show();
		m_render_context.SetClearColor( 0.0f, 0.0f, 0.5f );

		m_main_pipeline.SetShaders( m_vertex_shader, m_fragment_shader );
		m_main_pipeline.DescribeVertexLayout( vertex_layout );
	}

	void OnFrame( void ) override
	{
		m_window.PollEvents();
		m_render_context.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		m_texture_2d.Bind( 0 );
		m_triangle_vertex_buffer.Bind();
		m_main_pipeline.Bind();
		m_triangle_index_buffer.Bind();
		m_main_pipeline.Draw( m_triangle_index_buffer );
		m_main_pipeline.Unbind();

		m_render_context.SwapBuffers();
	}

	bool IsRunning( void ) override { return m_window.IsOpen(); }

private:

	Orbit::Window           m_window;
	Orbit::RenderContext    m_render_context;
	Orbit::VertexShader     m_vertex_shader;
	Orbit::FragmentShader   m_fragment_shader;
	Orbit::VertexBuffer     m_triangle_vertex_buffer;
	Orbit::IndexBuffer      m_triangle_index_buffer;
	Orbit::GraphicsPipeline m_main_pipeline;
	Orbit::Texture2D        m_texture_2d;
	float                   m_time;

};