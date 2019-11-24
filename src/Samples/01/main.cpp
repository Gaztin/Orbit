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

#include <cmath>
#include <ctime>

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/IO/Log.h>
#include <Orbit/Core/Utility/Utility.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Buffer/ConstantBuffer.h>
#include <Orbit/Graphics/Buffer/IndexBuffer.h>
#include <Orbit/Graphics/Buffer/VertexBuffer.h>
#include <Orbit/Graphics/Shader/FragmentShader.h>
#include <Orbit/Graphics/Shader/GraphicsPipeline.h>
#include <Orbit/Graphics/Shader/VertexShader.h>
#include <Orbit/Graphics/RenderContext.h>

ORB_APP_DECL( SampleApp )
{
public:
	SampleApp();

	void OnFrame() override;
	bool IsRunning() override { return !!m_window; }

	static void OnWindowEvent( const Orbit::WindowEvent& e );

private:
	Orbit::Window                  m_window;
	Orbit::Window::SubscriptionPtr m_window_subscription;
	Orbit::RenderContext           m_render_context;
	Orbit::VertexShader            m_vertex_shader;
	Orbit::FragmentShader          m_fragment_shader;
	Orbit::VertexBuffer            m_triangle_vertex_buffer;
	Orbit::IndexBuffer             m_triangle_index_buffer;
	Orbit::ConstantBuffer          m_triangle_constant_buffer;
	Orbit::GraphicsPipeline        m_main_pipeline;
	float                          m_time;
};

struct Vertex
{
	float x, y, z, w;
	float r, g, b, a;
};

const Orbit::VertexLayout vertex_layout
{
	{ "POSITION", Orbit::VertexComponent::Vec4 },
	{ "COLOR",    Orbit::VertexComponent::Vec4 },
};

const std::initializer_list< Vertex > triangle_vertices
{
	{ -0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f },
	{ -0.5f,  0.5f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f },
	{  0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f, 1.0f },
	{  0.5f,  0.5f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f },
};

const std::initializer_list< uint16_t > triangle_indices
{
	0, 1, 2,
	3, 2, 1,
};

std::tuple triangle_constants = std::make_tuple( 1.0f );

SampleApp::SampleApp()
	: m_window( 800, 600 )
	, m_window_subscription( m_window.subscribe( &SampleApp::OnWindowEvent ) )
	, m_render_context( m_window, Orbit::GraphicsAPI::OpenGL )
	, m_vertex_shader( Orbit::Asset( "shader.vs" ) )
	, m_fragment_shader( Orbit::Asset( "shader.fs" ) )
	, m_triangle_vertex_buffer( triangle_vertices )
	, m_triangle_index_buffer( triangle_indices )
	, m_triangle_constant_buffer( triangle_constants )
	, m_time( 0.0f )
{
	m_window.SetTitle( "Orbit sample #01" );
	m_window.Show();
	m_render_context.SetClearColor( 0.0f, 0.0f, 0.5f );

	m_main_pipeline.SetShaders( m_vertex_shader, m_fragment_shader );
	m_main_pipeline.DescribeVertexLayout( vertex_layout );

	/* Load text asset and log its contents */
	{
		Orbit::Asset test_asset( "text.txt" );
		const auto& txt = test_asset.GetData();
		Orbit::LogInfo( std::string( reinterpret_cast< const char* >( txt.data() ), txt.size() ) );
	}
}

void SampleApp::OnFrame()
{
	m_time = static_cast< float >( clock() ) / CLOCKS_PER_SEC;
	const float diffuse = 0.5f + ( 0.5f * sin( m_time * static_cast< float >( M_PI ) ) );

	std::get< 0 >( triangle_constants ) = diffuse;

	m_window.PollEvents();
	m_render_context.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

	m_triangle_vertex_buffer.Bind();
	m_main_pipeline.Bind();
	{
		m_triangle_index_buffer.Bind();
		m_triangle_constant_buffer.Bind( Orbit::ShaderType::Vertex, 0 );
		m_triangle_constant_buffer.Update( triangle_constants );
		m_main_pipeline.Draw( m_triangle_index_buffer );
	}
	m_main_pipeline.Unbind();

	m_render_context.SwapBuffers();
}

void SampleApp::OnWindowEvent( const Orbit::WindowEvent& e )
{
	switch( e.type )
	{
		case Orbit::WindowEvent::Resize:
			Orbit::LogInfo( Orbit::Format( "Resized: (%d, %d)", e.data.resize.w, e.data.resize.h ) );
			break;

		case Orbit::WindowEvent::Move:
			Orbit::LogInfo( Orbit::Format( "Moved: (%d, %d)", e.data.move.x, e.data.move.y ) );
			break;

		case Orbit::WindowEvent::Defocus:
			Orbit::LogInfo( "Defocus" );
			break;

		case Orbit::WindowEvent::Focus:
			Orbit::LogInfo( "Focus" );
			break;

		case Orbit::WindowEvent::Suspend:
			Orbit::LogInfo( "Suspend" );
			break;

		case Orbit::WindowEvent::Restore:
			Orbit::LogInfo( "Restore" );
			break;

		case Orbit::WindowEvent::Close:
			Orbit::LogInfo( "Close" );
			break;

		default:
			break;
	}
}
