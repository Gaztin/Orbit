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

#include "VertexShader.h"

#include "Orbit/Core/IO/Asset.h"
#include "Orbit/Core/IO/Log.h"
#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/API/OpenGL/GLSL.h"
#include "Orbit/Graphics/Device/RenderContext.h"

#if _ORB_HAS_GRAPHICS_API_D3D11
#  include <d3dcompiler.h>
#endif

ORB_NAMESPACE_BEGIN

VertexShader::VertexShader( const Asset& asset )
{
	auto& context_impl_var = RenderContext::GetCurrent()->GetPrivateImpl();

	switch( context_impl_var.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL

		case( unique_index_v< Private::_RenderContextImplOpenGL, Private::RenderContextImpl > ):
		{
			auto& impl         = m_impl.emplace< Private::_VertexShaderImplOpenGL >();
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( context_impl_var );

			const std::string_view version_directive = GLSL::GetVersionDirective( context_impl.opengl_version, context_impl.embedded );
			const std::string_view glsl_define       = GLSL::GetGLSLDefine();
			const std::string_view precision         = GLSL::GetPrecision( context_impl.embedded );
			const std::string_view constants_macros  = GLSL::GetConstantsMacros( context_impl.opengl_version, context_impl.embedded );
			const std::string_view varying_macro     = GLSL::GetVaryingMacro( context_impl.opengl_version, context_impl.embedded, ShaderType::Vertex );
			const std::string_view attribute_macro   = GLSL::GetAttributeMacro( context_impl.opengl_version, context_impl.embedded, ShaderType::Vertex );

			const GLchar* sources[]
			{
				version_directive.data(),
				glsl_define.data(),
				constants_macros.data(),
				precision.data(),
				varying_macro.data(),
				attribute_macro.data(),
				reinterpret_cast< const GLchar* >( asset.GetData() ),
			};

			const GLint lengths[] =
			{
				static_cast< GLint >( version_directive.size() ),
				static_cast< GLint >( glsl_define.size() ),
				static_cast< GLint >( constants_macros.size() ),
				static_cast< GLint >( precision.size() ),
				static_cast< GLint >( varying_macro.size() ),
				static_cast< GLint >( attribute_macro.size() ),
				static_cast< GLint >( asset.GetSize() ),
			};

			static_assert( count_of( sources ) == count_of( lengths ) );

			impl.id = context_impl.functions->create_shader( OpenGL::ShaderType::Vertex );
			context_impl.functions->shader_source( impl.id, static_cast< GLsizei >( count_of( sources ) ), sources, lengths );
			context_impl.functions->compile_shader( impl.id );

			GLint loglen = 0;
			context_impl.functions->get_shaderiv( impl.id, OpenGL::ShaderParam::InfoLogLength, &loglen );
			if( loglen > 0 )
			{
				std::string logbuf( static_cast< size_t >( loglen ), '\0' );
				context_impl.functions->get_shader_info_log( impl.id, loglen, nullptr, &logbuf[ 0 ] );
				LogError( logbuf );
			}

			break;
		}

	#endif
	#if _ORB_HAS_GRAPHICS_API_D3D11

		case( unique_index_v< Private::_RenderContextImplD3D11, Private::RenderContextImpl > ):
		{
			auto&                  impl         = m_impl.emplace< Private::_VertexShaderImplD3D11 >();
			auto&                  context_impl = std::get< Private::_RenderContextImplD3D11 >( context_impl_var );
			const D3D_SHADER_MACRO macros[]     = { { "ORB_HLSL", "1" }, { NULL, NULL } };
			UINT                   flags        = ( D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS );
		#if defined( _DEBUG )
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION );
		#else
			flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL3 );
		#endif

			ID3DBlob* vertex_data   = nullptr;
			ID3DBlob* vertex_errors = nullptr;
			if( D3DCompile( asset.GetData(), asset.GetSize(), NULL, macros, nullptr, "main", "vs_5_0", flags, 0, &vertex_data, &vertex_errors ) != S_OK )
			{
				LogError( Format( "%s", static_cast< const char* >( vertex_errors->GetBufferPointer() ) ) );
				vertex_errors->Release();
				return;
			}

			ID3D11VertexShader* vertex_shader;
			context_impl.device->CreateVertexShader( vertex_data->GetBufferPointer(), vertex_data->GetBufferSize(), nullptr, &vertex_shader );
			impl.vertex_data.reset( vertex_data );
			impl.vertex_shader.reset( vertex_shader );

			break;
		}

	#endif

	}
}

VertexShader::~VertexShader( void )
{
	switch( m_impl.index() )
	{
		default: break;

	#if _ORB_HAS_GRAPHICS_API_OPENGL

		case( unique_index_v< Private::_VertexShaderImplOpenGL, Private::VertexShaderImpl > ):
		{
			auto& impl         = std::get< Private::_VertexShaderImplOpenGL >( m_impl );
			auto& context_impl = std::get< Private::_RenderContextImplOpenGL >( RenderContext::GetCurrent()->GetPrivateImpl() );

			context_impl.functions->delete_shader( impl.id );

			break;
		}

	#endif

	}
}

ORB_NAMESPACE_END
