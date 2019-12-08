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
#include <Orbit/Graphics/Buffer/ConstantBuffer.h>
#include <Orbit/Graphics/Buffer/IndexBuffer.h>
#include <Orbit/Graphics/Buffer/Texture2D.h>
#include <Orbit/Graphics/Buffer/VertexBuffer.h>
#include <Orbit/Graphics/Device/RenderContext.h>
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

const Orbit::VertexLayout vertex_layout
{
	{ "POSITION", Orbit::VertexComponent::Vec4 },
	{ "COLOR",    Orbit::VertexComponent::Vec4 },
	{ "TEXCOORD", Orbit::VertexComponent::Vec2 },
};

constexpr std::string_view shader_source = R"(
#if defined( GLSL )

#  if defined( VERTEX )

ORB_CONSTANTS_BEGIN( Constants )
	ORB_CONSTANT( mat4, mvp );
ORB_CONSTANTS_END

ORB_ATTRIBUTE( 0 ) vec4 a_position;
ORB_ATTRIBUTE( 1 ) vec4 a_color;
ORB_ATTRIBUTE( 2 ) vec2 a_texcoord;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;

void main()
{
	v_position = mvp * a_position;
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

cbuffer Constants
{
	matrix mvp;
};

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
	output.position = mul( input.position, mvp );
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

std::tuple constant_data = std::make_tuple( Orbit::Matrix4() );

Orbit::Matrix4 projection_matrix( 0.f );

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
		, m_resize_subscription( m_window.Subscribe( OnWindowResize ) )
		, m_shader( shader_source, vertex_layout )
		, m_vertex_buffer( vertex_data )
		, m_index_buffer( index_data )
		, m_constant_buffer( constant_data )
		, m_texture( 4, 4, texture_data )
	{
		m_window.SetTitle( "Orbit Sample (02-Cube)" );
		m_window.Show();
		m_render_context.SetClearColor( 0.0f, 0.0f, 0.5f );
	}

public:

	void OnFrame( float delta_time ) override
	{
		/* Update constant buffer */
		{
			auto& [ mvp ] = constant_data;

			/* Calculate model-view-projection matrix */
			{
				using namespace Orbit::MathLiterals;
				using namespace Orbit::UnitLiterals::Metric;

				Orbit::Matrix4 view;
				view.Translate( Orbit::Vector3( 0_m, 0_m, 5_m ) );

				m_model.Rotate( Orbit::Vector3( 0_pi, 0.5_pi * delta_time, 0_pi ) );

				mvp = m_model * view * projection_matrix;
			}

			m_constant_buffer.Update( constant_data );
		}

		m_window.PollEvents();
		m_render_context.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		Orbit::RenderCommand command;
		command.vertex_buffer = &m_vertex_buffer;
		command.index_buffer  = &m_index_buffer;
		command.shader        = &m_shader;
		command.constant_buffers.push_back( &m_constant_buffer );
		command.textures.push_back( &m_texture );

		m_renderer.QueueCommand( command );
		m_renderer.Render();

		m_render_context.SwapBuffers();
	}

	bool IsRunning() override { return m_window.IsOpen(); }

public:

	static void OnWindowResize( const Orbit::WindowResized& e )
	{
		using namespace Orbit::MathLiterals;

		constexpr float fov       = 60_pi / 180.f;
		constexpr float far_clip  = 100.f;
		constexpr float near_clip = 0.1f;
		const float     aspect    = static_cast< float >( e.width ) / e.height;

		projection_matrix.SetPerspective( aspect, fov, near_clip, far_clip );
	}

private:

	Orbit::Window            m_window;
	Orbit::EventSubscription m_resize_subscription;
	Orbit::RenderContext     m_render_context;
	Orbit::Shader            m_shader;
	Orbit::VertexBuffer      m_vertex_buffer;
	Orbit::IndexBuffer       m_index_buffer;
	Orbit::ConstantBuffer    m_constant_buffer;
	Orbit::Texture2D         m_texture;
	Orbit::BasicRenderer     m_renderer;
	Orbit::Matrix4           m_model;

};
