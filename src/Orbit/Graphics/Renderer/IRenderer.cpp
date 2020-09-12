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

#include "IRenderer.h"

#include "Orbit/Graphics/API/OpenGL/OpenGLFunctions.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Buffer/VertexBuffer.h"
#include "Orbit/Graphics/Context/RenderContext.h"
#include "Orbit/Graphics/Renderer/RenderCommand.h"
#include "Orbit/Graphics/Shader/Shader.h"

ORB_NAMESPACE_BEGIN

#if( ORB_HAS_D3D11 )

constexpr D3D11_BLEND BlendFactorToD3D11( BlendFactor factor )
{
	// FIXME: I'm worried about the fact that both ConstantColor and ConstantAlpha evaluate to D3D11_BLEND_BLEND_FACTOR.
	//        Can this be fixed by filtering the BlendFactor we send into ID3D11DeviceContext::OMSetBlendState?

	switch( factor )
	{
		case Orbit::BlendFactor::Zero:                return D3D11_BLEND_ZERO;
		case Orbit::BlendFactor::One:                 return D3D11_BLEND_ONE;
		case Orbit::BlendFactor::SourceColor:         return D3D11_BLEND_SRC_COLOR;
		case Orbit::BlendFactor::SourceAlpha:         return D3D11_BLEND_SRC_ALPHA;
		case Orbit::BlendFactor::DestinationColor:    return D3D11_BLEND_DEST_COLOR;
		case Orbit::BlendFactor::DestinationAlpha:    return D3D11_BLEND_DEST_ALPHA;
		case Orbit::BlendFactor::ConstantColor:       return D3D11_BLEND_BLEND_FACTOR;
		case Orbit::BlendFactor::ConstantAlpha:       return D3D11_BLEND_BLEND_FACTOR;
		case Orbit::BlendFactor::SourceAlphaSaturate: return D3D11_BLEND_SRC_ALPHA_SAT;
		case Orbit::BlendFactor::InvSourceColor:      return D3D11_BLEND_INV_SRC_COLOR;
		case Orbit::BlendFactor::InvSourceAlpha:      return D3D11_BLEND_INV_SRC_ALPHA;
		case Orbit::BlendFactor::InvDestinationColor: return D3D11_BLEND_INV_DEST_COLOR;
		case Orbit::BlendFactor::InvDestinationAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
		case Orbit::BlendFactor::InvConstantColor:    return D3D11_BLEND_INV_BLEND_FACTOR;
		case Orbit::BlendFactor::InvConstantAlpha:    return D3D11_BLEND_INV_BLEND_FACTOR;
		default:                                      return static_cast< D3D11_BLEND >( 0 );
	}
}

constexpr D3D11_BLEND_OP BlendOpToD3D11( BlendOp op )
{
	switch( op )
	{
		case Orbit::BlendOp::Add:         return D3D11_BLEND_OP_ADD;
		case Orbit::BlendOp::Subtract:    return D3D11_BLEND_OP_SUBTRACT;
		case Orbit::BlendOp::RevSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
		case Orbit::BlendOp::Min:         return D3D11_BLEND_OP_MIN;
		case Orbit::BlendOp::Max:         return D3D11_BLEND_OP_MAX;
		default:                          return static_cast< D3D11_BLEND_OP >( 0 );
	}
}

#endif // ORB_HAS_D3D11
#if( ORB_HAS_OPENGL )

constexpr OpenGLBlendFactor BlendFactorToGL( BlendFactor factor )
{
	switch( factor )
	{
		case Orbit::BlendFactor::Zero:                return OpenGLBlendFactor::Zero;
		case Orbit::BlendFactor::One:                 return OpenGLBlendFactor::One;
		case Orbit::BlendFactor::SourceColor:         return OpenGLBlendFactor::SrcColor;
		case Orbit::BlendFactor::SourceAlpha:         return OpenGLBlendFactor::SrcAlpha;
		case Orbit::BlendFactor::DestinationColor:    return OpenGLBlendFactor::DstColor;
		case Orbit::BlendFactor::DestinationAlpha:    return OpenGLBlendFactor::DstAlpha;
		case Orbit::BlendFactor::ConstantColor:       return OpenGLBlendFactor::ConstantColor;
		case Orbit::BlendFactor::ConstantAlpha:       return OpenGLBlendFactor::ConstantAlpha;
		case Orbit::BlendFactor::SourceAlphaSaturate: return OpenGLBlendFactor::SrcAlphaSaturate;
		case Orbit::BlendFactor::InvSourceColor:      return OpenGLBlendFactor::OneMinusSrcColor;
		case Orbit::BlendFactor::InvSourceAlpha:      return OpenGLBlendFactor::OneMinusSrcAlpha;
		case Orbit::BlendFactor::InvDestinationColor: return OpenGLBlendFactor::OneMinusDstColor;
		case Orbit::BlendFactor::InvDestinationAlpha: return OpenGLBlendFactor::OneMinusDstAlpha;
		case Orbit::BlendFactor::InvConstantColor:    return OpenGLBlendFactor::OneMinusConstantColor;
		case Orbit::BlendFactor::InvConstantAlpha:    return OpenGLBlendFactor::OneMinusConstantAlpha;
		default:                                      return static_cast< OpenGLBlendFactor >( ~0 );
	}
}

constexpr OpenGLBlendMode BlendOpToGL( BlendOp op )
{
	switch( op )
	{
		case Orbit::BlendOp::Add:         return OpenGLBlendMode::Add;
		case Orbit::BlendOp::Subtract:    return OpenGLBlendMode::Subtract;
		case Orbit::BlendOp::RevSubtract: return OpenGLBlendMode::ReverseSubtract;
		case Orbit::BlendOp::Min:         return OpenGLBlendMode::Min;
		case Orbit::BlendOp::Max:         return OpenGLBlendMode::Max;
		default:                          return static_cast< OpenGLBlendMode >( 0 );
	}
}

#endif // ORB_HAS_OPENGL


void IRenderer::PushCommand( RenderCommand command )
{
	commands_.emplace_back( std::move( command ) );
}

void IRenderer::APIDraw( const RenderCommand& command )
{
	auto& context_details = RenderContext::GetInstance().GetPrivateDetails();

	switch( context_details.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )

		case( unique_index_v< Private::_RenderContextDetailsD3D11, Private::RenderContextDetails > ):
		{
			auto& d3d11 = std::get< Private::_RenderContextDetailsD3D11 >( context_details );

			switch( command.topology )
			{
				case Topology::Points:    { d3d11.device_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST    ); } break;
				case Topology::Lines:     { d3d11.device_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST     ); } break;
				case Topology::Triangles: { d3d11.device_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ); } break;
			}

			if( command.blend_enabled )
			{
				auto it = d3d11.blend_states.find( command.blend_equation );
				if( it == d3d11.blend_states.end() )
				{
					D3D11_BLEND_DESC desc;
					desc.AlphaToCoverageEnable                   = FALSE;
					desc.IndependentBlendEnable                  = FALSE;
					desc.RenderTarget[ 0 ].BlendEnable           = TRUE;
					desc.RenderTarget[ 0 ].SrcBlend              = BlendFactorToD3D11( command.blend_equation.src_factor_color );
					desc.RenderTarget[ 0 ].DestBlend             = BlendFactorToD3D11( command.blend_equation.dst_factor_color );
					desc.RenderTarget[ 0 ].BlendOp               = BlendOpToD3D11( command.blend_equation.op_color );
					desc.RenderTarget[ 0 ].SrcBlendAlpha         = BlendFactorToD3D11( command.blend_equation.src_factor_alpha );
					desc.RenderTarget[ 0 ].DestBlendAlpha        = BlendFactorToD3D11( command.blend_equation.dst_factor_alpha );
					desc.RenderTarget[ 0 ].BlendOpAlpha          = BlendOpToD3D11( command.blend_equation.op_alpha );
					desc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

					ComPtr< ID3D11BlendState > blend_state;
					d3d11.device->CreateBlendState( &desc, &blend_state.ptr_ );
					it = d3d11.blend_states.try_emplace( d3d11.blend_states.end(), command.blend_equation, std::move( blend_state ) );
				}

				d3d11.device_context->OMSetBlendState( it->second.ptr_, &command.blend_equation.constant[ 0 ], 0xFFFFFFFF );
			}
			else
			{
				d3d11.device_context->OMSetBlendState( d3d11.disable_blending_blend_state.ptr_, NULL, 0xFFFFFFFF );
			}

			if( command.index_buffer ) d3d11.device_context->DrawIndexed( static_cast< UINT >( command.index_buffer->GetCount() ), 0, 0 );
			else                       d3d11.device_context->Draw( static_cast< UINT >( command.vertex_buffer->GetCount() ), 0 );

		} break;

	#endif
	#if( ORB_HAS_OPENGL )

		case( unique_index_v< Private::_RenderContextDetailsOpenGL, Private::RenderContextDetails > ):
		{
			OpenGLDrawMode draw_mode  = { };

			switch( command.topology )
			{
				case Topology::Points:    { draw_mode = OpenGLDrawMode::Points;    } break;
				case Topology::Lines:     { draw_mode = OpenGLDrawMode::Lines;     } break;
				case Topology::Triangles: { draw_mode = OpenGLDrawMode::Triangles; } break;
			}

			if( command.blend_enabled )
			{
				glEnable( GL_BLEND );

				if( command.blend_equation.src_factor_color == command.blend_equation.src_factor_alpha )
					glBlendFunc(
						BlendFactorToGL( command.blend_equation.src_factor_color ),
						BlendFactorToGL( command.blend_equation.dst_factor_color )
					);
				else
					glBlendFuncSeparate(
						BlendFactorToGL( command.blend_equation.src_factor_color ),
						BlendFactorToGL( command.blend_equation.dst_factor_color ),
						BlendFactorToGL( command.blend_equation.src_factor_alpha ),
						BlendFactorToGL( command.blend_equation.dst_factor_alpha )
					);

				if( command.blend_equation.op_color == command.blend_equation.op_alpha )
					glBlendEquation(
						BlendOpToGL( command.blend_equation.op_color )
					);
				else
					glBlendEquationSeparate(
						BlendOpToGL( command.blend_equation.op_color ),
						BlendOpToGL( command.blend_equation.op_alpha )
					);
			}
			else
			{
				glDisable( GL_BLEND );
			}

			if( command.index_buffer )
			{
				OpenGLIndexType index_type = { };

				switch( command.index_buffer->GetFormat() )
				{
					case IndexFormat::Byte:       { index_type = OpenGLIndexType::Byte;  } break;
					case IndexFormat::Word:       { index_type = OpenGLIndexType::Short; } break;
					case IndexFormat::DoubleWord: { index_type = OpenGLIndexType::Int;   } break;
				}

				glDrawElements( draw_mode, static_cast< GLsizei >( command.index_buffer->GetCount() ), index_type, nullptr );
			}
			else
			{
				glDrawArrays( draw_mode, 0, static_cast< GLsizei >( command.vertex_buffer->GetCount() ) );
			}

		} break;

	#endif

	}
}

ORB_NAMESPACE_END
