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
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/Shape/CubeShape.h>
#include <Orbit/Core/Time/Clock.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Geometry/MeshFactory.h>
#include <Orbit/Graphics/Geometry/Mesh.h>
#include <Orbit/Graphics/Renderer/DefaultRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Graphics/Texture/Texture.h>

static const std::string shader_source = R"(
cbuffer VertexUniforms
{
	float4x4 view_projection;
	float4x4 model;
};

cbuffer PixelUniforms
{
	float4x4 view_projection2;
	float4x4 model2;
};

struct VertexData
{
	float4 Position : POSITION;
	float3 Normal   : NORMAL;
};

struct PixelData
{
	float4 Position : SV_POSITION;
	float4 WPos     : POSITION0;
	float3 Normal   : NORMAL;
};

PixelData VSMain( VertexData input )
{
	PixelData data;
	data.Position = mul( mul( input.Position, model ), view_projection );
	data.WPos     = input.Position;
	data.Normal   = input.Normal;
	return data;
}

float4 PSMain( PixelData input ) : SV_TARGET0
{
	input.WPos.xyz /= input.WPos.w;

	float4 CenterOnScreen = mul( mul( float4( 0.0, 0.0, 0.0, 1.0 ), model2 ), view_projection2 );
	CenterOnScreen.xyz   /= CenterOnScreen.w;

	float4 Unit           = mul( mul( float4( 1.0, 1.0, 0.0, 1.0 ), model2 ), view_projection2 );
	Unit.xyz             /= Unit.w;
	float  UnitLength     = length( Unit.xy );

	float4 PixelOnScreen  = mul( mul( input.WPos, model2 ), view_projection2 );
	PixelOnScreen.xyz    /= PixelOnScreen.w;

	float2 Diff           = ( PixelOnScreen.xy - CenterOnScreen.xy ) / Unit.xy;
	float  Dist           = length( Diff );

	clip( 0.5 - Dist );

	float3 SyntheticNormal = normalize( input.WPos.xyz - float3( 0.0, 0.0, 0.0 ) );
	float3 LightDir        = normalize( float3( 0.4, -0.8, -0.2 ) );
	float  Dot             = dot( SyntheticNormal, -LightDir );

	return float4( Dot.xxx, 1.0 );
}
)";

static Orbit::VertexLayout vl
{
	Orbit::VertexComponent::Position,
	Orbit::VertexComponent::Normal,
};

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: shader_ ( shader_source, vl )
		, mesh_   ( Orbit::MeshFactory::GetInstance().CreateMeshFromShape( Orbit::CubeShape( 1.0f ), vl ) )
		, texture_( Orbit::Asset( "textures/checkerboard.tga" ) )
	{
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
		camera_.position.x = 2.000f;
		camera_.position.z = 2.000f;
		camera_.position.y = 1.500f;
		camera_.rotation.x = 0.175f * Orbit::Pi;
		camera_.rotation.y = 0.750f * Orbit::Pi;
	}

public:

	void OnFrame( void ) override
	{
		const float delta_time = Orbit::Clock::GetDelta();

		// Rotate cube
//		model_matrix_.Rotate( Orbit::Vector3( 0.0f, 0.5f * Orbit::Pi * delta_time, 0.0f ) );

		// Update window and clear context
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		// Update camera
		camera_.Update( delta_time );

		// Update uniforms
		const auto model = mesh_.transform_ * model_matrix_;
		const auto vp    = camera_.GetViewProjection();
		shader_.SetVertexUniform( "view_projection",  &vp,    sizeof( Orbit::Matrix4 ) );
		shader_.SetVertexUniform( "model",            &model, sizeof( Orbit::Matrix4 ) );
		shader_.SetPixelUniform(  "view_projection2", &vp,    sizeof( Orbit::Matrix4 ) );
		shader_.SetPixelUniform(  "model2",           &model, sizeof( Orbit::Matrix4 ) );

		// Push cube mesh to render queue
		Orbit::RenderCommand command;
		command.vertex_buffer = mesh_.GetVertexBuffer();
		command.index_buffer  = mesh_.GetIndexBuffer();
		command.shader        = shader_;
		command.textures.emplace_back( texture_.GetTexture2D() );
		Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );

		// Render scene
		Orbit::DefaultRenderer::GetInstance().Render();

		// Swap buffers
		render_context_.SwapBuffers();
	}

private:

	Orbit::RenderContext  render_context_;
	Orbit::Shader         shader_;
	Orbit::Mesh           mesh_;
	Orbit::Texture        texture_;
	Orbit::Matrix4        model_matrix_;
	Camera                camera_;

};
