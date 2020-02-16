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

#include "Camera.h"

#include <Orbit/Core/Input/Input.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Math/Vector3.h>

Camera::Camera( void )
	: on_resize_{ Orbit::Window::GetInstance().Subscribe( [ this ]( const Orbit::WindowResized& e ){ OnResized( e ); } ) }
{
	position.z = -5.0f;
}

void Camera::Update( float delta_time )
{
	UpdateView( delta_time );

	if( Orbit::Input::GetPointerPressed( Orbit::Input::pointer_index_mouse_right ) )
	{
		Orbit::Input::SetFPSCursor( true );
	}

	if( Orbit::Input::GetPointerReleased( Orbit::Input::pointer_index_mouse_right ) )
	{
		Orbit::Input::SetFPSCursor( false );
	}
}

void Camera::OnResized( const Orbit::WindowResized& e )
{
	width_  = e.width;
	height_ = e.height;
}

void Camera::UpdateView( float delta_time )
{
	/* Rotate */
	{
		if( Orbit::Input::GetPointerHeld( Orbit::Input::pointer_index_mouse_left ) || Orbit::Input::GetPointerHeld( Orbit::Input::pointer_index_mouse_right ) )
		{
			auto [ x, y ] = Orbit::Input::GetPointerMove( 0 );

			this->rotation.x += (  static_cast< float >( y ) * Orbit::Pi ) / height_;
			this->rotation.y += ( -static_cast< float >( x ) * Orbit::Pi ) / height_;
		}
	}

	/* Translate */
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
			rotation_matrix.Rotate( this->rotation );

			const float step = ( this->speed * speed_modifier * delta_time );

			this->position += ( rotation_matrix.right   * direction.x * step );
			this->position += ( rotation_matrix.up      * direction.y * step );
			this->position += ( rotation_matrix.forward * direction.z * step );
		}
	}
}

Orbit::Matrix4 Camera::GetViewProjection( void ) const
{
	const float fov_rad = ( this->fov * Orbit::Pi ) / 180.0f;
	const float aspect  = static_cast< float >( width_ ) / static_cast< float >( height_ );

	Orbit::Matrix4 view_rotate;
	view_rotate.Rotate( this->rotation );

	Orbit::Matrix4 view_translate;
	view_translate.Translate( this->position );

	Orbit::Matrix4 view_inverse( view_rotate * view_translate );
	view_inverse.Invert();

	Orbit::Matrix4 projection;
	projection.SetPerspective( aspect, fov_rad, this->near_clip, this->far_clip );

	return view_inverse * projection;
}
