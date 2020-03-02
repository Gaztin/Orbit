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

	camera_position_.z = -5.0f;
}

void TetraCubes::OnFrame( float delta_time )
{
	window_.PollEvents();
	context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

	HandleInput( delta_time );

	// Render cube
	{
		Orbit::Matrix4 view_rotate;
		view_rotate.Rotate( camera_rotation_ );

		Orbit::Matrix4 view_translate;
		view_translate.Translate( camera_position_ );

		CubeConstants constants;
		constants.view_projection = ( ( view_rotate * view_translate ).Inverted() * projection_matrix_ );
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

	window_width_  = width;
	window_height_ = height;

	projection_matrix_.SetPerspective( aspect, fov, near_clip, far_clip );
}

void TetraCubes::HandleInput( float delta_time )
{
	const float move_distance = ( 4.0f * delta_time );

	// Camera rotation
	if( Orbit::Input::GetPointerHeld( Orbit::Input::pointer_index_mouse_left ) || Orbit::Input::GetPointerHeld( Orbit::Input::pointer_index_mouse_right ) )
	{
		const Orbit::Point mouse_movement = Orbit::Input::GetPointerMove( 0 );
		camera_rotation_.x += ( ( Orbit::Pi * mouse_movement.y ) / window_height_ );
		camera_rotation_.y -= ( ( Orbit::Pi * mouse_movement.x ) / window_width_  );
	}

	// Camera movement
	{
		Orbit::Vector3 direction;
		float          speed_modifier = 1.0f;

		if( Orbit::Input::GetKeyHeld( Orbit::Key::A       ) ) { direction.x -= 1.0f; }
		if( Orbit::Input::GetKeyHeld( Orbit::Key::D       ) ) { direction.x += 1.0f; }
		if( Orbit::Input::GetKeyHeld( Orbit::Key::C       ) ) { direction.y -= 1.0f; }
		if( Orbit::Input::GetKeyHeld( Orbit::Key::Space   ) ) { direction.y += 1.0f; }
		if( Orbit::Input::GetKeyHeld( Orbit::Key::S       ) ) { direction.z -= 1.0f; }
		if( Orbit::Input::GetKeyHeld( Orbit::Key::W       ) ) { direction.z += 1.0f; }
		if( Orbit::Input::GetKeyHeld( Orbit::Key::Shift   ) ) { speed_modifier *= 10.0f; }
		if( Orbit::Input::GetKeyHeld( Orbit::Key::Control ) ) { speed_modifier /= 10.0f; }

		direction.Normalize();

		if( direction.DotProduct() > 0.0f )
		{
			Orbit::Matrix4 rotation_matrix;
			rotation_matrix.Rotate( camera_rotation_ );

			const float step = ( speed_modifier * delta_time );

			camera_position_ += ( rotation_matrix.right   * direction.x * step );
			camera_position_ += ( rotation_matrix.up      * direction.y * step );
			camera_position_ += ( rotation_matrix.forward * direction.z * step );
		}
	}

	// Toggle FPS cursor
	if( Orbit::Input::GetPointerPressed(  Orbit::Input::pointer_index_mouse_right ) ) Orbit::Input::SetFPSCursor( true );
	if( Orbit::Input::GetPointerReleased( Orbit::Input::pointer_index_mouse_right ) ) Orbit::Input::SetFPSCursor( false );
}
