/*
 * Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include <orbit/core/events/window_event.h>
#include <orbit/core/application.h>
#include <orbit/core/asset.h>
#include <orbit/core/entry_point.h>
#include <orbit/core/log.h>
#include <orbit/core/utility.h>
#include <orbit/core/window.h>
#include <orbit/graphics/constant_buffer.h>
#include <orbit/graphics/fragment_shader.h>
#include <orbit/graphics/graphics_pipeline.h>
#include <orbit/graphics/index_buffer.h>
#include <orbit/graphics/render_context.h>
#include <orbit/graphics/vertex_buffer.h>
#include <orbit/graphics/vertex_shader.h>

class sample_app final : public orb::application< sample_app >
{
public:
	sample_app();

	void frame();
	bool is_running() { return !!m_window; }

	static void on_window_event( const orb::window_event& e );

private:
	orb::window                   m_window;
	orb::window::subscription_ptr m_windowSubscription;
	orb::render_context           m_renderContext;
	orb::vertex_shader            m_vertexShader;
	orb::fragment_shader          m_fragmentShader;
	orb::vertex_buffer            m_triangleVertexBuffer;
	orb::index_buffer             m_triangleIndexBuffer;
	orb::constant_buffer          m_triangleConstantBuffer;
	orb::graphics_pipeline        m_mainPipeline;
	float                         m_time;
};

struct vertex
{
	float x, y, z, w;
	float r, g, b, a;
};

const orb::vertex_layout vertexLayout =
{
	{ "POSITION", orb::vertex_component::Vec4 },
	{ "COLOR",    orb::vertex_component::Vec4 },
};

const std::initializer_list< vertex > triangleVertices =
{
	{ -0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f },
	{ -0.5f,  0.5f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f },
	{  0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f, 1.0f },
	{  0.5f,  0.5f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f },
};

const std::initializer_list< uint16_t > triangleIndices =
{
	0, 1, 2,
	3, 2, 1,
};

std::tuple triangleConstants = std::make_tuple
(
	1.0f
);

sample_app::sample_app()
	: m_window( 800, 600 )
	, m_windowSubscription( m_window.subscribe( &sample_app::on_window_event ) )
	, m_renderContext( m_window, orb::graphics_api::OpenGL )
	, m_vertexShader( orb::asset( "shader.vs" ) )
	, m_fragmentShader( orb::asset( "shader.fs" ) )
	, m_triangleVertexBuffer( triangleVertices )
	, m_triangleIndexBuffer( triangleIndices )
	, m_triangleConstantBuffer( triangleConstants )
	, m_time( 0.0f )
{
	m_window.set_title( "Orbit sample #01" );
	m_window.show();
	m_renderContext.set_clear_color( 0.0f, 0.0f, 0.5f );

	m_mainPipeline.set_shaders( m_vertexShader, m_fragmentShader );
	m_mainPipeline.describe_vertex_layout( vertexLayout );

	/* Load text asset and log its contents */
	{
		orb::asset testAsset( "text.txt" );
		const auto& txt = testAsset.get_data();
		orb::log_info( std::string( reinterpret_cast< const char* >( txt.data() ), txt.size() ) );
	}
}

void sample_app::frame()
{
	m_time = static_cast< float >( clock() ) / CLOCKS_PER_SEC;
	const float diffuse = 0.5f + ( 0.5f * sin( m_time * static_cast< float >( M_PI ) ) );

	std::get< 0 >( triangleConstants ) = diffuse;

	m_window.poll_events();
	m_renderContext.clear( orb::buffer_mask::Color | orb::buffer_mask::Depth );

	m_triangleVertexBuffer.bind();
	m_mainPipeline.bind();
	{
		m_triangleIndexBuffer.bind();
		m_triangleConstantBuffer.bind( orb::shader_type::Vertex, 0 );
		m_triangleConstantBuffer.update( triangleConstants );
		m_mainPipeline.draw( m_triangleIndexBuffer );
	}
	m_mainPipeline.unbind();

	m_renderContext.swap_buffers();
}

void sample_app::on_window_event( const orb::window_event& e )
{
	switch( e.type )
	{
		case orb::window_event::Resize:
			orb::log_info( orb::format( "Resized: (%d, %d)", e.data.resize.w, e.data.resize.h ) );
			break;

		case orb::window_event::Move:
			orb::log_info( orb::format( "Moved: (%d, %d)", e.data.move.x, e.data.move.y ) );
			break;

		case orb::window_event::Defocus:
			orb::log_info( "Defocus" );
			break;

		case orb::window_event::Focus:
			orb::log_info( "Focus" );
			break;

		case orb::window_event::Suspend:
			orb::log_info( "Suspend" );
			break;

		case orb::window_event::Restore:
			orb::log_info( "Restore" );
			break;

		case orb::window_event::Close:
			orb::log_info( "Close" );
			break;

		default:
			break;
	}
}
