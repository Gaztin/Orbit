/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
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

#include "Shader.h"

#include "Orbit/Core/IO/Log.h"
#include "Orbit/Graphics/Buffer/IndexBuffer.h"
#include "Orbit/Graphics/Device/RenderContext.h"

#if( ORB_HAS_D3D11 )
#  include <d3dcompiler.h>
#endif

ORB_NAMESPACE_BEGIN

Shader::Shader( std::string_view source, const VertexLayout& vertex_layout )
{
	Private::RenderContextImpl& context_impl = RenderContext::GetCurrent()->GetPrivateImpl();

	switch( context_impl.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )
		case( unique_index_v< Private::_RenderContextImplD3D11, Private::RenderContextImpl > ):
		{
			Private::_RenderContextImplD3D11& d3d11 = std::get< Private::_RenderContextImplD3D11 >( context_impl );
			Private::_ShaderImplD3D11&        impl  = m_impl.emplace< Private::_ShaderImplD3D11 >();

			UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS;

		#if defined( _DEBUG )
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION );
		#else
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL3 );
		#endif

			const D3D_SHADER_MACRO macros[]
			{
				"HLSL",  "1",
				nullptr, nullptr,
			};

			ID3DBlob* vertex_data = nullptr;

			/* Create vertex shader */
			{
				ID3DBlob* errors = nullptr;

				if( FAILED( D3DCompile( source.data(), source.size(), nullptr, macros, nullptr, "VSMain", "vs_5_0", flags, 0, &vertex_data, &errors ) ) )
				{
					LogError( Format( "%s", errors->GetBufferPointer() ) );
					errors->Release();
				}

				ID3D11VertexShader* vertex_shader = nullptr;

				if( SUCCEEDED( d3d11.device->CreateVertexShader( vertex_data->GetBufferPointer(), vertex_data->GetBufferSize(), nullptr, &vertex_shader ) ) )
				{
					impl.vertex_shader.reset( vertex_shader );
				}
			}

			/* Create pixel shader */
			{
				ID3DBlob* pixel_data;
				ID3DBlob* errors;

				if( FAILED( D3DCompile( source.data(), source.size(), nullptr, macros, nullptr, "PSMain", "ps_5_0", flags, 0, &pixel_data, &errors ) ) )
				{
					LogError( Format( "%s", errors->GetBufferPointer() ) );
					errors->Release();
				}

				ID3D11PixelShader* pixel_shader;

				if( SUCCEEDED( d3d11.device->CreatePixelShader( pixel_data->GetBufferPointer(), pixel_data->GetBufferSize(), nullptr, &pixel_shader ) ) )
				{
					pixel_data->Release();
					impl.pixel_shader.reset( pixel_shader );
				}
			}

			/* Create input layout */
			if( vertex_data )
			{
				std::vector< D3D11_INPUT_ELEMENT_DESC > descriptors;

				for( const VertexComponent& component : vertex_layout )
				{
					D3D11_INPUT_ELEMENT_DESC desc { };
					desc.SemanticName      = component.semantic_name.c_str();
					desc.AlignedByteOffset = descriptors.empty() ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
					desc.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

					switch( component.type )
					{
						default:                     { desc.Format = DXGI_FORMAT_UNKNOWN;            } break;
						case VertexComponent::Float: { desc.Format = DXGI_FORMAT_R32_FLOAT;          } break;
						case VertexComponent::Vec2:  { desc.Format = DXGI_FORMAT_R32G32_FLOAT;       } break;
						case VertexComponent::Vec3:  { desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;    } break;
						case VertexComponent::Vec4:  { desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; } break;
					}

					descriptors.push_back( desc );
				}

				ID3D11InputLayout* input_layout;

				if( SUCCEEDED( d3d11.device->CreateInputLayout( descriptors.data(), static_cast< UINT >( descriptors.size() ), vertex_data->GetBufferPointer(), vertex_data->GetBufferSize(), &input_layout ) ) )
				{
					impl.input_layout.reset( input_layout );
				}

				vertex_data->Release();
			}

			/* Create sampler state */
			{
				D3D11_SAMPLER_DESC desc { };
				desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				desc.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.MaxAnisotropy    = 1;
				desc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
				desc.MinLOD           = 0.0f;
				desc.MaxLOD           = D3D11_FLOAT32_MAX;

				ID3D11SamplerState* sampler_state;

				if( SUCCEEDED( d3d11.device->CreateSamplerState( &desc, &sampler_state ) ) )
				{
					impl.sampler_state.reset( sampler_state );
				}
			}

			break;
		}
	#endif

	#if( ORB_HAS_OPENGL )
	#endif

	}
}

void Shader::Bind( void )
{
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )
		case( unique_index_v< Private::_ShaderImplD3D11, Private::ShaderImpl > ):
		{
			Private::_RenderContextImplD3D11& d3d11 = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );
			Private::_ShaderImplD3D11&        impl  = std::get< Private::_ShaderImplD3D11 >( m_impl );

			if( impl.input_layout )
				d3d11.device_context->IASetInputLayout( impl.input_layout.get() );

			if( impl.vertex_shader )
				d3d11.device_context->VSSetShader( impl.vertex_shader.get(), nullptr, 0 );

			if( impl.pixel_shader )
				d3d11.device_context->PSSetShader( impl.pixel_shader.get(), nullptr, 0 );

			if( impl.sampler_state )
			{
				ID3D11SamplerState* sampler_states[] = { impl.sampler_state.get() };
				d3d11.device_context->PSSetSamplers( 0, 1, sampler_states );
			}

			break;
		}
	#endif

	}
}

void Shader::Unbind( void )
{
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )
		case( unique_index_v< Private::_ShaderImplD3D11, Private::ShaderImpl > ):
		{
			Private::_RenderContextImplD3D11& d3d11 = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );
			Private::_ShaderImplD3D11&        impl  = std::get< Private::_ShaderImplD3D11 >( m_impl );

			if( impl.input_layout )
			{
				ID3D11InputLayout* bound_input_layout;
				d3d11.device_context->IAGetInputLayout( &bound_input_layout );
				if( bound_input_layout == impl.input_layout.get() )
				{
					d3d11.device_context->IASetInputLayout( nullptr );
					bound_input_layout->Release();
				}
			}

			if( impl.vertex_shader )
			{
				ID3D11VertexShader* bound_vertex_shader;
				d3d11.device_context->VSGetShader( &bound_vertex_shader, nullptr, nullptr );
				if( bound_vertex_shader == impl.vertex_shader.get() )
				{
					d3d11.device_context->VSSetShader( nullptr, nullptr, 0 );
					bound_vertex_shader->Release();
				}
			}

			if( impl.pixel_shader )
			{
				ID3D11PixelShader* bound_pixel_shader;
				d3d11.device_context->PSGetShader( &bound_pixel_shader, nullptr, nullptr );
				if( bound_pixel_shader == impl.pixel_shader.get() )
				{
					d3d11.device_context->PSSetShader( nullptr, nullptr, 0 );
					bound_pixel_shader->Release();
				}
			}

			if( impl.sampler_state )
			{
				ID3D11SamplerState* bound_sampler_state;
				d3d11.device_context->PSGetSamplers( 0, 1, &bound_sampler_state );
				if( bound_sampler_state == impl.sampler_state.get() )
				{
					d3d11.device_context->PSSetSamplers( 0, 0, nullptr );
					bound_sampler_state->Release();
				}
			}

			break;
		}
	#endif

	}
}

void Shader::Draw( const IndexBuffer& ib )
{
	switch( m_impl.index() )
	{
		default: break;

	#if( ORB_HAS_D3D11 )
		case( unique_index_v< Private::_ShaderImplD3D11, Private::ShaderImpl > ):
		{
			Private::_RenderContextImplD3D11& d3d11 = std::get< Private::_RenderContextImplD3D11 >( RenderContext::GetCurrent()->GetPrivateImpl() );

			d3d11.device_context->DrawIndexed( static_cast< UINT >( ib.GetCount() ), 0, 0 );

			break;
		}
	#endif

	}
}

ORB_NAMESPACE_END
