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
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Model/Model.h>
#include <Orbit/Graphics/Renderer/BasicRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Math/Literals.h>
#include <Orbit/Math/Matrix4.h>
#include <Orbit/Math/Vector2.h>
#include <Orbit/Math/Vector3.h>
#include <Orbit/Math/Vector4.h>

#include "Framework/Camera.h"
#include "ModelData.h"

const Orbit::VertexLayout vertex_layout
{
	Orbit::VertexComponent::Position,
	Orbit::VertexComponent::Color,
	Orbit::VertexComponent::TexCoord,
	Orbit::VertexComponent::Normal,
};

constexpr std::string_view shader_source = R"(
#if defined( GLSL )

#  if defined( VERTEX )

ORB_CONSTANTS_BEGIN( VertexConstants )
	ORB_CONSTANT( mat4, view_projection );
	ORB_CONSTANT( mat4, model );
	ORB_CONSTANT( mat4, model_inverse );
ORB_CONSTANTS_END

ORB_ATTRIBUTE( 0 ) vec4 a_position;
ORB_ATTRIBUTE( 1 ) vec4 a_color;
ORB_ATTRIBUTE( 2 ) vec2 a_texcoord;
ORB_ATTRIBUTE( 3 ) vec3 a_normal;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;
ORB_VARYING vec3 v_normal;

void main()
{
	v_position = view_projection * model * a_position;
	v_color    = a_color;
	v_texcoord = a_texcoord;
	v_normal   = ( transpose( model_inverse ) * vec4( a_normal, 1.0 ) ).xyz;

	gl_Position = v_position;
}

#  elif defined( FRAGMENT )

ORB_CONSTANTS_BEGIN( FragmentConstants )
	ORB_CONSTANT( vec3, light_dir );
ORB_CONSTANTS_END

uniform sampler2D diffuse_texture;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;
ORB_VARYING vec3 v_normal;

void main()
{
	vec4 tex_color = texture( diffuse_texture, v_texcoord );
	vec4 out_color = tex_color + v_color;

	float diffuse  = -dot( v_normal, light_dir );
	out_color.rgb *= diffuse;

	ORB_SET_OUT_COLOR( out_color );
}

#  endif

#elif defined( HLSL )

Texture2D diffuse_texture;

SamplerState texture_sampler;

cbuffer VertexConstants
{
	matrix view_projection;
	matrix model;
	matrix model_inverse;
};

cbuffer PixelConstants
{
	float3 light_dir;
};

struct VertexData
{
	float4 position : POSITION;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
	float3 normal   : NORMAL;
};

struct PixelData
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
	float3 normal   : NORMAL;
};

PixelData VSMain( VertexData input )
{
	PixelData output;
	output.position = mul( input.position, mul( model, view_projection ) );
	output.color    = input.color;
	output.texcoord = input.texcoord;
	output.normal   = mul( float4( input.normal, 1.0 ), transpose( model_inverse ) ).xyz;

	return output;
}

float4 PSMain( PixelData input ) : SV_TARGET
{
	float4 tex_color = diffuse_texture.Sample( texture_sampler, input.texcoord );
	float4 out_color = tex_color + input.color;

	float  diffuse  = -dot( input.normal, light_dir );
	out_color.rgb  *= diffuse;

	return out_color;
}

#endif
)";

std::tuple vertex_constant_data   = std::make_tuple( Orbit::Matrix4(), Orbit::Matrix4(), Orbit::Matrix4() );
std::tuple fragment_constant_data = std::make_tuple( Orbit::Vector3() );

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
		, m_render_context( Orbit::GraphicsAPI::OpenGL )
		, m_shader( shader_source, vertex_layout )
		, m_model( model_data, vertex_layout )
		, m_vertex_constant_buffer( vertex_constant_data )
		, m_fragment_constant_buffer( fragment_constant_data )
		, m_texture( 4, 4, texture_data )
	{
		m_window.SetTitle( "Orbit Sample (03-Model)" );
		m_window.Show();
		m_render_context.SetClearColor( 0.0f, 0.0f, 0.5f );
		m_model_matrix.Translate( Orbit::Vector3( 0.0f, -2.0f, 0.0f ) );
	}

public:

	void OnFrame( float delta_time ) override
	{
		/* Update constant buffers */
		{
			auto& [ view_projection, model, model_inverse ] = vertex_constant_data;
			auto& [ light_dir ]                             = fragment_constant_data;

			/* Calculate model-view-projection matrix */
			{
				using namespace Orbit::MathLiterals;

				m_model_matrix.Rotate( Orbit::Vector3( 0_pi, 0.5_pi * delta_time, 0_pi ) );

				view_projection = m_camera.GetViewProjection();
				model           = m_model_matrix;
				model_inverse   = m_model_matrix;
				model_inverse.Invert();
			}

			/* Light position */
			light_dir = Orbit::Vector3( 1.0f, -1.0f, 1.0f );

			m_vertex_constant_buffer.Update( vertex_constant_data );
			m_fragment_constant_buffer.Update( fragment_constant_data );
		}

		m_window.PollEvents();
		m_render_context.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		m_camera.Update( delta_time );

		Orbit::RenderCommand command = m_model.MakeRenderCommand();
		command.shader = &m_shader;
		command.constant_buffers[ Orbit::ShaderType::Vertex   ].push_back( &m_vertex_constant_buffer );
		command.constant_buffers[ Orbit::ShaderType::Fragment ].push_back( &m_fragment_constant_buffer );
		command.textures.push_back( &m_texture );

		m_renderer.QueueCommand( command );
		m_renderer.Render();

		m_render_context.SwapBuffers();
	}

	bool IsRunning() override { return m_window.IsOpen(); }

private:

	Orbit::Window            m_window;
	Orbit::RenderContext     m_render_context;
	Orbit::Shader            m_shader;
	Orbit::Model             m_model;
	Orbit::ConstantBuffer    m_vertex_constant_buffer;
	Orbit::ConstantBuffer    m_fragment_constant_buffer;
	Orbit::Texture2D         m_texture;
	Orbit::BasicRenderer     m_renderer;
	Orbit::Matrix4           m_model_matrix;

	Camera m_camera;

};