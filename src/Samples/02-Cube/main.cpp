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

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: shader_ ( shader_source_.Generate(), shader_source_.GetVertexLayout() )
		, mesh_   ( Orbit::MeshFactory::GetInstance().CreateMeshFromShape( Orbit::CubeShape( 1.0f ), shader_source_.GetVertexLayout() ) )
		, texture_( Orbit::Asset( "textures/checkerboard.tga" ) )
	{
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
		camera_.position.y = 2.000f;
		camera_.rotation.x = 0.125f * Orbit::Pi;
	}

public:

	void OnFrame( void ) override
	{
		const float delta_time = Orbit::Clock::GetDelta();

		// Rotate cube
		model_matrix_.Rotate( Orbit::Vector3( 0.0f, 0.5f * Orbit::Pi * delta_time, 0.0f ) );

		// Update window and clear context
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		// Update camera
		camera_.Update( delta_time );

		// Update uniforms
		const Orbit::Matrix4 model = mesh_.transform_ * model_matrix_;
		shader_.SetVertexUniform( shader_source_.u_view_projection, camera_.GetViewProjection() );
		shader_.SetVertexUniform( shader_source_.u_model,           model );
		shader_.SetVertexUniform( shader_source_.u_model_inverse,   model.Inverted() );

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
	CubeShader            shader_source_;
	Orbit::Shader         shader_;
	Orbit::Mesh           mesh_;
	Orbit::Texture        texture_;
	Orbit::Matrix4        model_matrix_;
	Camera                camera_;

};
