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

#include "TriangleShader.h"

#include <Orbit/Core/Application/Application.h>
#include <Orbit/Core/Application/EntryPoint.h>
#include <Orbit/Core/IO/Asset.h>
#include <Orbit/Core/Widget/Window.h>
#include <Orbit/Graphics/Buffer/IndexBuffer.h>
#include <Orbit/Graphics/Buffer/VertexBuffer.h>
#include <Orbit/Graphics/Context/RenderContext.h>
#include <Orbit/Graphics/Geometry/Geometry.h>
#include <Orbit/Graphics/Renderer/DefaultRenderer.h>
#include <Orbit/Graphics/Shader/Shader.h>
#include <Orbit/Graphics/Texture/Texture.h>
#include <Orbit/Math/Vector/Vector2.h>
#include <Orbit/Math/Vector/Vector4.h>

class SampleApp final : public Orbit::Application< SampleApp >
{
public:

	SampleApp( void )
		: window_  ( 800, 600 )
		, shader_  ( shader_source_.Generate(), shader_source_.GetVertexLayout() )
		, mesh_    ( "Triangle" )
		, texture_ ( Orbit::Asset( "textures/checkerboard.tga" ) )
		, time_    ( 0.0f )
	{
		CreateTriangleMesh();

		window_.SetTitle( "Orbit Sample (01-Triangle)" );
		window_.Show();
		render_context_.SetClearColor( 0.0f, 0.0f, 0.5f );
	}

	void OnFrame( float /*delta_time*/ ) override
	{
		window_.PollEvents();
		render_context_.Clear( Orbit::BufferMask::Color | Orbit::BufferMask::Depth );

		Orbit::RenderCommand command;
		command.vertex_buffer = mesh_.GetVertexBuffer();
		command.index_buffer  = mesh_.GetIndexBuffer();
		command.shader        = shader_;
		command.textures.emplace_back( texture_.GetTexture2D() );

		Orbit::DefaultRenderer::GetInstance().PushCommand( std::move( command ) );
		Orbit::DefaultRenderer::GetInstance().Render();

		render_context_.SwapBuffers();
	}

	bool IsRunning( void ) override { return window_.IsOpen(); }

private:

	void CreateTriangleMesh( void )
	{
		Orbit::Geometry geometry( shader_source_.GetVertexLayout() );
		Orbit::Face     face;
		Orbit::Vertex   vertex;

		// Bottom left corner
		vertex.position   = Orbit::Vector4( -1.0f / Orbit::PythagorasConstant, -1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f );
		vertex.color      = Orbit::Color( 1.0f, 0.0f, 1.0f, 1.0f );
		vertex.tex_coord  = Orbit::Vector2( 0.0f, 0.0f );
		face.indices[ 0 ] = geometry.AddVertex( vertex );

		// Top center corner
		vertex.position   = Orbit::Vector4(  0.0f,                              1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f );
		vertex.color      = Orbit::Color( 0.0f, 1.0f, 1.0f, 1.0f );
		vertex.tex_coord  = Orbit::Vector2( 0.5f, 1.0f );
		face.indices[ 1 ] = geometry.AddVertex( vertex );

		// Bottom right corner
		vertex.position   = Orbit::Vector4(  1.0f / Orbit::PythagorasConstant, -1.0f / Orbit::PythagorasConstant, 0.0f, 1.0f );
		vertex.color      = Orbit::Color( 1.0f, 1.0f, 0.0f, 1.0f );
		vertex.tex_coord  = Orbit::Vector2( 1.0f, 0.0f );
		face.indices[ 2 ] = geometry.AddVertex( vertex );

		// Create face
		geometry.AddFace( face );

		// Generate mesh
		mesh_ = geometry.ToMesh( "Triangle" );
	}

private:

	Orbit::Window        window_;
	Orbit::RenderContext render_context_;
	TriangleShader       shader_source_;
	Orbit::Shader        shader_;
	Orbit::Mesh          mesh_;
	Orbit::Texture       texture_;
	float                time_;

};
