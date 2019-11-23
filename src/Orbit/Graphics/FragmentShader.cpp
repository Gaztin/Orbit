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

#include "FragmentShader.h"

#include "Orbit/Core/Asset.h"
#include "Orbit/Core/Log.h"
#include "Orbit/Core/Utility.h"
#include "Orbit/Graphics/Platform/OpenGL/GLSL.h"
#include "Orbit/Graphics/RenderContext.h"

#if _ORB_HAS_GRAPHICS_API_D3D11
#  include <d3dcompiler.h>
#endif

ORB_NAMESPACE_BEGIN

template< typename T >
constexpr auto render_context_impl_index_v = unique_index_v< T, RenderContextImpl >;

template< typename T >
constexpr auto fragment_shader_impl_index_v = unique_index_v< T, FragmentShaderImpl >;

FragmentShader::FragmentShader( const Asset& asset )
{
	auto context_impl_ptr = RenderContext::GetCurrent()->GetImplPtr();

	switch( context_impl_ptr->index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( render_context_impl_index_v< _RenderContextImplOpenGL > ):
		{
			auto  impl         = std::addressof( m_impl.emplace< _FragmentShaderImplOpenGL >() );
			auto  context_impl = std::get_if< _RenderContextImplOpenGL >( context_impl_ptr );

			const std::string_view version_directive = GLSL::GetVersionDirective( context_impl->opengl_version, context_impl->embedded );
			const std::string_view glsl_define       = GLSL::GetGLSLDefine();
			const std::string_view precision         = GLSL::GetPrecision( context_impl->embedded );
			const std::string_view constants_macros  = GLSL::GetConstantsMacros( context_impl->opengl_version, context_impl->embedded );
			const std::string_view varying_macro     = GLSL::GetVaryingMacro( context_impl->opengl_version, context_impl->embedded, ShaderType::Fragment );
			const std::string_view out_color_macro   = GLSL::GetOutColorMacro( context_impl->opengl_version, context_impl->embedded );
			const auto&            shader_data       = asset.GetData();

			const GLchar* sources[]
			{
				version_directive.data(),
				glsl_define.data(),
				constants_macros.data(),
				precision.data(),
				varying_macro.data(),
				out_color_macro.data(),
				reinterpret_cast< const GLchar* >( shader_data.data() ),
			};

			const GLint lengths[]
			{
				static_cast< GLint >( version_directive.size() ),
				static_cast< GLint >( glsl_define.size() ),
				static_cast< GLint >( constants_macros.size() ),
				static_cast< GLint >( precision.size() ),
				static_cast< GLint >( varying_macro.size() ),
				static_cast< GLint >( out_color_macro.size() ),
				static_cast< GLint >( shader_data.size() ),
			};

			static_assert( count_of( sources ) == count_of( lengths ) );

			impl->id = context_impl->functions->create_shader( OpenGL::ShaderType::Fragment );
			context_impl->functions->shader_source( impl->id, static_cast< GLsizei >( count_of( sources ) ), sources, lengths );
			context_impl->functions->compile_shader( impl->id );

			GLint loglen = 0;
			context_impl->functions->get_shaderiv( impl->id, OpenGL::ShaderParam::InfoLogLength, &loglen );
			if( loglen > 0 )
			{
				std::string logbuf( static_cast< size_t >( loglen ), '\0' );
				context_impl->functions->get_shader_info_log( impl->id, loglen, nullptr, &logbuf[ 0 ] );
				LogError( logbuf );
			}

			break;
		}
	#endif

	#if _ORB_HAS_GRAPHICS_API_D3D11
		case( render_context_impl_index_v< _RenderContextImplD3D11 > ):
		{
			auto                   impl         = std::addressof( m_impl.emplace< _FragmentShaderImplD3D11 >() );
			auto                   context_impl = std::get_if< _RenderContextImplD3D11 >( context_impl_ptr );
			const auto&            data         = asset.GetData();
			const D3D_SHADER_MACRO macros[]     = { { "ORB_HLSL", "1" }, { NULL, NULL } };
			UINT                   flags        = ( D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS );
		#if defined( _DEBUG )
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION );
		#else
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL3 );
		#endif

			ID3DBlob* pixel_data   = nullptr;
			ID3DBlob* pixel_errors = nullptr;
			if( D3DCompile( data.data(), data.size(), NULL, macros, nullptr, "main", "ps_5_0", flags, 0, &pixel_data, &pixel_errors ) != S_OK )
			{
				LogError( Format( "%s", static_cast< const char* >( pixel_errors->GetBufferPointer() ) ) );
				pixel_errors->Release();
				return;
			}

			ID3D11PixelShader* pixelShader;
			context_impl->device->CreatePixelShader( pixel_data->GetBufferPointer(), pixel_data->GetBufferSize(), nullptr, &pixelShader );
			impl->pixel_shader.reset( pixelShader );
			pixel_data->Release();

			break;
		}
	#endif
	}
}

FragmentShader::~FragmentShader()
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL
		case( fragment_shader_impl_index_v< _FragmentShaderImplOpenGL > ):
		{
			auto impl         = std::get_if< _FragmentShaderImplOpenGL >( &m_impl );
			auto context_impl = std::get_if< _RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetImplPtr() );

			context_impl->functions->delete_shader( impl->id );

			break;
		}
	#endif
	}
}

ORB_NAMESPACE_END
