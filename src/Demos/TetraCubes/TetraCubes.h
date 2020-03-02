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

#pragma once
#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Buffer/ConstantBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Model/Mesh.h>
#include <Orbit/Graphics/Renderer/BasicRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Math/Matrix4.h>

class TetraCubes : public Orbit::Application< TetraCubes >
{
public:

	TetraCubes( void );

public:

	void OnFrame  ( float delta_time ) override;
	bool IsRunning( void )             override;

private:

	struct CubeConstants
	{
		Orbit::Matrix4 view_projection;
		Orbit::Matrix4 model;
	};

private:

	void OnResize   ( uint32_t width, uint32_t height );
	void HandleInput( float delta_time );

private:

	Orbit::Window         window_;
	Orbit::RenderContext  context_;
	Orbit::Shader         cube_shader_;
	Orbit::ConstantBuffer cube_constant_buffer_;
	Orbit::BasicRenderer  renderer_;
	Orbit::Mesh           cube_mesh_;

	Orbit::Matrix4 projection_matrix_;
	Orbit::Matrix4 model_matrix_;

	Orbit::EventSubscription on_resize_;

	Orbit::Vector3 camera_rotation_;
	Orbit::Vector3 camera_position_;

	uint32_t window_width_;
	uint32_t window_height_;

};
