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

#include "TetraCubes.h"

#include "CubeShader.h"

#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/Input/Input.h>
#include <Orbit/Core/Shape/CubeShape.h>
#include <Orbit/Graphics/Model/MeshFactory.h>

static CubeShader cube_shader;

TetraCubes::TetraCubes( void )
	: window_              ( 768, 768 )
	, context_             ( Orbit::default_graphics_api )
	, cube_shader_         ( cube_shader )
	, cube_constant_buffer_( sizeof( CubeConstants ) )
	, cube_mesh_           ( Orbit::MeshFactory::GetInstance().CreateMeshFromShape( Orbit::CubeShape( 0.5f ), cube_shader.GetVertexLayout() ) )
	, on_resize_           ( window_.Subscribe( [ this ]( const Orbit::WindowResized& e ){ OnResize( e.width, e.height ); } ) )
{
	window_.Show();
	context_.SetClearColor( 0.1f, 0.1f, 0.1f );

	view_matrix_.pos.z -= 5.0f;
}

void TetraCubes::OnFrame( float delta_time )
{
	window_.PollEvents();
	context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

	model_matrix_.RotateY( delta_time * Orbit::Pi );

	HandleInput();

	// Render cube
	{
		CubeConstants constants;
		constants.view_projection = ( view_matrix_.Inverted() * projection_matrix_ );
		constants.model           = cube_mesh_.transform * model_matrix_;
		cube_constant_buffer_.Update( &constants, sizeof( CubeConstants ) );

		Orbit::RenderCommand command;
		command.vertex_buffer = *cube_mesh_.vertex_buffer;
		command.index_buffer  = *cube_mesh_.index_buffer;
		command.shader        = cube_shader_;
		command.constant_buffers[ Orbit::ShaderType::Vertex ].emplace_back( cube_constant_buffer_ );

		renderer_.QueueCommand( command );
	}

	renderer_.Render();
	context_.SwapBuffers();
}

bool TetraCubes::IsRunning( void )
{
	return window_.IsOpen();
}

void TetraCubes::OnResize( uint32_t width, uint32_t height )
{
	const float aspect    = ( static_cast< float >( width ) / static_cast< float >( height ) );
	const float fov       = ( 60.0f * Orbit::Pi / 180.0f );
	const float near_clip = 0.1f;
	const float far_clip  = 100.0f;

	projection_matrix_.SetPerspective( aspect, fov, near_clip, far_clip );
}

void TetraCubes::HandleInput( void )
{
	if( Orbit::Input::GetKeyPressed( Orbit::Key::_1 ) )
		model_matrix_.pos.y -= 0.5f;

	if( Orbit::Input::GetKeyPressed( Orbit::Key::_2 ) )
		model_matrix_.pos.y += 0.5f;
}
