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
#include <Orbit/Core/Shape/SphereShape.h>
#include <Orbit/Core/Utility/PredefinedColors.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Buffer/ConstantBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Debug/DebugManager.h>
#include <Orbit/Graphics/Geometry/MeshFactory.h>
#include <Orbit/Graphics/Geometry/Mesh.h>
#include <Orbit/Graphics/Renderer/DefaultRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Graphics/Texture/Texture.h>
#include <Orbit/Math/Geometry/Plane.h>
#include <Orbit/Math/Literals.h>
#include <Orbit/Math/Matrix/Matrix4.h>
#include <Orbit/Math/Vector/Vector2.h>
#include <Orbit/Math/Vector/Vector3.h>
#include <Orbit/Math/Vector/Vector4.h>

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
		, plane_rotation_ ( 0.0f, 0.0f, 0.5f * Orbit::Pi )
		, play_time_      ( 0.0f )
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
		// Rotate plane with arrow keys
		if( Orbit::Input::GetKeyHeld( Orbit::Key::ArrowLeft ) )
			plane_rotation_.z += delta_time;
		if( Orbit::Input::GetKeyHeld( Orbit::Key::ArrowRight ) )
			plane_rotation_.z -= delta_time;
		if( Orbit::Input::GetKeyHeld( Orbit::Key::ArrowUp ) )
			plane_rotation_.x += delta_time;
		if( Orbit::Input::GetKeyHeld( Orbit::Key::ArrowDown ) )
			plane_rotation_.x -= delta_time;

		// Press '1' to slice mesh in half
		if( Orbit::Input::GetKeyPressed( Orbit::Key::_1 ) )
		{
			Orbit::Matrix4 plane_matrix;
			plane_matrix.Rotate( plane_rotation_ );

			const Orbit::Plane plane( plane_matrix.up, 0.0f );

			auto old_sliced_meshes = std::move( sliced_meshes_ );

			if( old_sliced_meshes.empty() )
			{
				sliced_meshes_ = mesh_.Slice( plane );
			}
			else
			{
				for( size_t i = 0; i < old_sliced_meshes.size(); ++i )
				{
					auto new_sliced_meshes = old_sliced_meshes[ i ].Slice( plane );

					for( size_t m = 0; m < new_sliced_meshes.size(); ++m )
					{
						sliced_meshes_.emplace_back( std::move( new_sliced_meshes[ m ] ) );
					}
				}
			}

			play_time_ = 0.0f;
		}

		// Draw lines to show where plane is
		{
			Orbit::Matrix4 plane_matrix;
			plane_matrix.Rotate( plane_rotation_ );
			plane_matrix.Scale( Orbit::Vector3( 1.5f ) );

			Orbit::DebugManager::GetInstance().PushLineSegment( Orbit::LineSegment( -plane_matrix.right + -plane_matrix.forward, -plane_matrix.right +  plane_matrix.forward ), Orbit::Magenta );
			Orbit::DebugManager::GetInstance().PushLineSegment( Orbit::LineSegment( -plane_matrix.right +  plane_matrix.forward,  plane_matrix.right +  plane_matrix.forward ), Orbit::Magenta );
			Orbit::DebugManager::GetInstance().PushLineSegment( Orbit::LineSegment(  plane_matrix.right +  plane_matrix.forward,  plane_matrix.right + -plane_matrix.forward ), Orbit::Magenta );
			Orbit::DebugManager::GetInstance().PushLineSegment( Orbit::LineSegment(  plane_matrix.right + -plane_matrix.forward, -plane_matrix.right + -plane_matrix.forward ), Orbit::Magenta );
		}

		// Press 'R' to reset
		if( Orbit::Input::GetKeyPressed( Orbit::Key::R ) )
		{
			sliced_meshes_.clear();
		}

		/* Update constant buffer */
		{
			constant_data.view_projection = camera_.GetViewProjection();
		}

		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		camera_.Update( delta_time );

		if( sliced_meshes_.empty() )
		{
			constant_data.model           = ( mesh_.transform * model_matrix_ );
			constant_data.model_inverse   = constant_data.model.Inverted();
			constant_buffer_.Update( &constant_data, sizeof( ConstantData ) );

			Orbit::RenderCommand command;
			command.vertex_buffer = mesh_.GetVertexBuffer();
			command.index_buffer  = mesh_.GetIndexBuffer();
			command.shader        = shader_;
			command.constant_buffers[ Orbit::ShaderType::Vertex ].emplace_back( constant_buffer_ );
			command.textures.emplace_back( texture_.GetTexture2D() );
			
			Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );
			Orbit::DefaultRenderer::GetInstance().Render();
		}
		else
		{
			play_time_ = std::min( play_time_ + delta_time, 1.0f );

			for( int i = 0; i < sliced_meshes_.size(); ++i )
			{
				sliced_meshes_[ i ].transform.pos.x = std::sinf( play_time_ * Orbit::Pi * 0.5f ) * ( i == 0 ? -1 : 1 );
			}

			Orbit::RenderCommand command;
			command.shader = shader_;
			command.constant_buffers[ Orbit::ShaderType::Vertex ].emplace_back( constant_buffer_ );
			command.textures.emplace_back( texture_.GetTexture2D() );

			for( const Orbit::Mesh& mesh : sliced_meshes_ )
			{
				constant_data.model           = ( mesh.transform * model_matrix_ );
				constant_data.model_inverse   = constant_data.model.Inverted();
				constant_buffer_.Update( &constant_data, sizeof( ConstantData ) );

				command.vertex_buffer = mesh.GetVertexBuffer();
				command.index_buffer  = mesh.GetIndexBuffer();
				
				Orbit::DefaultRenderer::GetInstance().PushCommand( command );
				Orbit::DefaultRenderer::GetInstance().Render();
			}
		}

		Orbit::DebugManager::GetInstance().Render( Orbit::DefaultRenderer::GetInstance(), constant_data.view_projection );
		Orbit::DebugManager::GetInstance().Flush();

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

	std::vector< Orbit::Mesh > sliced_meshes_;
	Orbit::Vector3             plane_rotation_;

	Camera camera_;

	float play_time_;

};
