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
#include <Orbit/Core/Shape/CubeShape.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Buffer/ConstantBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Geometry/MeshFactory.h>
#include <Orbit/Graphics/Geometry/Mesh.h>
#include <Orbit/Graphics/Renderer/DefaultRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Graphics/Texture/Texture.h>
#include <Orbit/Math/Literals.h>
#include <Orbit/Math/Matrix4.h>
#include <Orbit/Math/Vector2.h>
#include <Orbit/Math/Vector3.h>
#include <Orbit/Math/Vector4.h>

static CubeShader cube_shader;

struct ConstantData
{
	Orbit::Matrix4 view_projection;
	Orbit::Matrix4 model;
	Orbit::Matrix4 model_inverse;

} constant_data;

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: window_         ( 800, 600 )
		, shader_         ( cube_shader.Generate(), cube_shader.GetVertexLayout() )
		, mesh_           ( Orbit::MeshFactory::GetInstance().CreateMeshFromShape( Orbit::CubeShape( 1.0f ), cube_shader.GetVertexLayout() ) )
		, constant_buffer_( sizeof( ConstantData ) )
		, texture_        ( Orbit::Asset( "textures/checkerboard.tga" ) )
	{
		window_.SetTitle( "Orbit Sample (02-Cube)" );
		window_.Show();
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
		camera_.position.y = 2.000f;
		camera_.rotation.x = 0.125f * Orbit::Pi;
	}

public:

	void OnFrame( float delta_time ) override
	{
		/* Update constant buffer */
		{
			model_matrix_.Rotate( Orbit::Vector3( 0.0f, 0.5f * Orbit::Pi * delta_time, 0.0f ) );

			constant_data.view_projection = camera_.GetViewProjection();
			constant_data.model           = ( mesh_.transform * model_matrix_ );
			constant_data.model_inverse   = constant_data.model.Inverted();
			constant_buffer_.Update( &constant_data, sizeof( ConstantData ) );
		}

		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		camera_.Update( delta_time );

		Orbit::RenderCommand command;
		command.vertex_buffer = *mesh_.vertex_buffer;
		command.index_buffer  = *mesh_.index_buffer;
		command.shader        = shader_;
		command.constant_buffers[ Orbit::ShaderType::Vertex ].emplace_back( constant_buffer_ );
		command.textures.emplace_back( texture_.GetTexture2D() );

		Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );
		Orbit::DefaultRenderer::GetInstance().Render();

		render_context_.SwapBuffers();
	}

	bool IsRunning( void ) override { return window_.IsOpen(); }

private:

	Orbit::Window         window_;
	Orbit::RenderContext  render_context_;
	Orbit::Shader         shader_;
	Orbit::Mesh           mesh_;
	Orbit::ConstantBuffer constant_buffer_;
	Orbit::Texture        texture_;
	Orbit::Matrix4        model_matrix_;

	Camera camera_;

};
