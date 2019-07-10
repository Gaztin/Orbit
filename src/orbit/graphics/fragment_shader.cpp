/*
* Copyright (c) 2018 Sebastian Kylander https://gaztin.com/
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

#include "fragment_shader.h"

#include "orbit/core/asset.h"
#include "orbit/core/log.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/platform/opengl/glsl.h"
#include "orbit/graphics/render_context.h"

#if __ORB_HAS_GRAPHICS_API_D3D11
#  include <d3dcompiler.h>
#endif

namespace orb
{
	template< typename T >
	constexpr auto render_context_impl_index_v = unique_index_v< T, render_context_impl >;

	template< typename T >
	constexpr auto fragment_shader_impl_index_v = unique_index_v< T, fragment_shader_impl >;

	fragment_shader::fragment_shader( const asset& ast )
	{
		auto currentContextImpl = render_context::get_current()->get_impl_ptr();
		switch( currentContextImpl->index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				auto  impl      = std::addressof( m_impl.emplace< __fragment_shader_impl_opengl >() );
				auto  implCtx   = std::get_if< __render_context_impl_opengl >( currentContextImpl );
				auto& functions = implCtx->functions.value();
				impl->id        = functions.create_shader( gl::shader_type::Fragment );

				const std::string_view versionDirectiveString = glsl::get_version_directive( implCtx->glVersion, implCtx->embedded );
				const std::string_view glslDefineString       = glsl::get_glsl_define();
				const std::string_view precisionString        = glsl::get_precision( implCtx->embedded );
				const std::string_view constantsMacrosString  = glsl::get_constants_macros( implCtx->glVersion, implCtx->embedded );
				const std::string_view varyingMacroString     = glsl::get_varying_macro( implCtx->glVersion, implCtx->embedded, shader_type::Fragment );
				const std::string_view outColorMacroString    = glsl::get_out_color_macro( implCtx->glVersion, implCtx->embedded );
				const auto&            shaderData             = ast.get_data();

				const GLchar* sources[] =
				{
					versionDirectiveString.data(),
					glslDefineString.data(),
					constantsMacrosString.data(),
					precisionString.data(),
					varyingMacroString.data(),
					outColorMacroString.data(),
					reinterpret_cast< const GLchar* >( shaderData.data() ),
				};

				const GLint lengths[] =
				{
					static_cast< GLint >( versionDirectiveString.size() ),
					static_cast< GLint >( glslDefineString.size() ),
					static_cast< GLint >( constantsMacrosString.size() ),
					static_cast< GLint >( precisionString.size() ),
					static_cast< GLint >( varyingMacroString.size() ),
					static_cast< GLint >( outColorMacroString.size() ),
					static_cast< GLint >( shaderData.size() ),
				};

				static_assert( count_of( sources ) == count_of( lengths ) );

				functions.shader_source( impl->id, static_cast< GLsizei >( count_of( sources ) ), sources, lengths );
				functions.compile_shader( impl->id );

				GLint loglen = 0;
				functions.get_shaderiv( impl->id, gl::shader_param::InfoLogLength, &loglen );
				if( loglen > 0 )
				{
					std::string logbuf( static_cast< size_t >( loglen ), '\0' );
					functions.get_shader_info_log( impl->id, loglen, nullptr, &logbuf[ 0 ] );
					log_error( logbuf );
				}

				break;
			}
		#endif

		#if __ORB_HAS_GRAPHICS_API_D3D11
			case( render_context_impl_index_v< __render_context_impl_d3d11 > ):
			{
				auto                   impl     = std::addressof( m_impl.emplace< __fragment_shader_impl_d3d11 >() );
				const auto&            data     = ast.get_data();
				const D3D_SHADER_MACRO macros[] = { { "ORB_HLSL", "1" }, { NULL, NULL } };
				UINT                   flags    = ( D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS );
			#if defined( _DEBUG )
				flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION );
			#else
				flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL3 );
			#endif

				ID3DBlob* pixelData   = nullptr;
				ID3DBlob* pixelErrors = nullptr;
				if( D3DCompile( data.data(), data.size(), NULL, macros, nullptr, "main", "ps_5_0", flags, 0, &pixelData, &pixelErrors ) != S_OK )
				{
					log_error( format( "%s", static_cast< const char* >( pixelErrors->GetBufferPointer() ) ) );
					pixelErrors->Release();
					return;
				}

				ID3D11Device&      device = *( std::get_if< __render_context_impl_d3d11 >( currentContextImpl )->device );
				ID3D11PixelShader* pixelShader;
				device.CreatePixelShader( pixelData->GetBufferPointer(), pixelData->GetBufferSize(), nullptr, &pixelShader );
				impl->pixelShader.reset( pixelShader );
				pixelData->Release();

				break;
			}
		#endif
		}
	}

	fragment_shader::~fragment_shader()
	{
		switch( m_impl.index() )
		{
			default: break;

		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( fragment_shader_impl_index_v< __fragment_shader_impl_opengl > ):
			{
				auto  impl      = std::get_if< __fragment_shader_impl_opengl >( &m_impl );
				auto& functions = std::get_if< __render_context_impl_opengl >( render_context::get_current()->get_impl_ptr() )->functions.value();

				functions.delete_shader( impl->id );

				break;
			}
		#endif
		}
	}
}
