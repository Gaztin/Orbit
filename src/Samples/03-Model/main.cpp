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
#include "ModelShader.h"

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/Time/Clock.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Geometry/Mesh.h>
#include <Orbit/Graphics/ModelFormats/COLLADAFile.h>
#include <Orbit/Graphics/Renderer/DefaultRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Graphics/Texture/Texture.h>

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: shader_ ( shader_source_.Generate(), shader_source_.GetVertexLayout() )
		, meshes_ ( Orbit::COLLADAFile( Orbit::Asset( "models/suzanne.dae" ), shader_source_.GetVertexLayout() ).GetModelData().meshes )
		, texture_( Orbit::Asset( "textures/checkerboard.tga" ) )
	{
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
	}

public:

	void OnFrame( void ) override
	{
		const float delta_time = Orbit::Clock::GetDelta();

		// Clear context
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		// Update camera
		camera_.Update( delta_time );

		// Rotate model
		model_matrix_.Rotate( Orbit::Vector3( 0.0f, 0.5f * Orbit::Pi * delta_time, 0.0f ) );

		// Update vertex uniforms
		shader_.SetVertexUniform( shader_source_.u_view_projection, camera_.GetViewProjection() );
		shader_.SetVertexUniform( shader_source_.u_model,           model_matrix_ );
		shader_.SetVertexUniform( shader_source_.u_model_inverse,   model_matrix_.Inverted() );

		// Update pixel uniforms
		shader_.SetPixelUniform( shader_source_.u_light_dir, Orbit::Vector3( 1.0f, -1.0f, 1.0f ).Normalized() );

		// Push meshes to render queue
		for( auto& mesh : meshes_ )
		{
			Orbit::RenderCommand command;
			command.vertex_buffer = mesh->GetVertexBuffer();
			command.index_buffer  = mesh->GetIndexBuffer();
			command.shader        = shader_;
			command.textures.emplace_back( texture_.GetTexture2D() );
			Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );
		}

		// Render scene
		Orbit::DefaultRenderer::GetInstance().Render();

		// Swap buffers
		render_context_.SwapBuffers();
	}

private:

	Orbit::RenderContext                          render_context_;
	ModelShader                                   shader_source_;
	Orbit::Shader                                 shader_;
	std::vector< std::shared_ptr< Orbit::Mesh > > meshes_;
	Orbit::Texture                                texture_;
	Orbit::Matrix4                                model_matrix_;
	Camera                                        camera_;

};
