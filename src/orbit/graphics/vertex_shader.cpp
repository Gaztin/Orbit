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

#include "vertex_shader.h"

#include "orbit/core/asset.h"
#include "orbit/core/log.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/render_context.h"

#if __ORB_HAS_GRAPHICS_API_D3D11
  #include <d3dcompiler.h>
#endif

namespace orb
{
	template< typename T >
	constexpr auto render_context_impl_index_v = unique_index_v< T, render_context_impl >;

	template< typename T >
	constexpr auto vertex_shader_impl_index_v = unique_index_v< T, vertex_shader_impl >;

	vertex_shader::vertex_shader( const asset& ast )
	{
		auto currentContextImpl = render_context::get_current()->get_impl_ptr();
		switch( currentContextImpl->index() )
		{
		#if __ORB_HAS_GRAPHICS_API_OPENGL
			case( render_context_impl_index_v< __render_context_impl_opengl > ):
			{
				auto  impl      = std::addressof( m_impl.emplace< __vertex_shader_impl_opengl >() );
				auto  implCtx   = std::get_if< __render_context_impl_opengl >( currentContextImpl );
				auto& functions = implCtx->functions.value();
				impl->id        = functions.create_shader( gl::shader_type::Vertex );

				std::string_view versionString;
				std::string_view precisionString;
				if( implCtx->embedded )
				{
					precisionString = "precision highp float;\n";

					switch( implCtx->version )
					{
						default:
						case version( 2 ): versionString = "#version 100\n";    break;
						case version( 3 ): versionString = "#version 300\n";    break;
//						case version( 3 ): versionString = "#version 300 es\n"; break;
					}
				}
				else
				{
					switch( implCtx->version )
					{
						default:
						case version( 2, 0 ):  versionString = "#version 110\n"; break;
						case version( 2, 1 ):  versionString = "#version 120\n"; break;
						case version( 3, 0 ):  versionString = "#version 130\n"; break;
						case version( 3, 1 ):  versionString = "#version 140\n"; break;
						case version( 3, 2 ):  versionString = "#version 150\n"; break;
						case version( 3, 3 ):  versionString = "#version 330\n"; break;
						case version( 4, 0 ):  versionString = "#version 400\n"; break;
						case version( 4, 1 ):  versionString = "#version 410\n"; break;
						case version( 4, 2 ):  versionString = "#version 420\n"; break;
						case version( 4, 3 ):  versionString = "#version 430\n"; break;
					}
				}

				/* GLES 3 or GL 3.1+ supports uniform buffer objects */
				std::string_view constantsMacrosString;
				if( ( implCtx->embedded && implCtx->version >= version( 3 ) ) || ( implCtx->version >= version( 3, 1 ) ) )
					constantsMacrosString = "#define ORB_CONSTANTS_BEGIN(X) layout (std140) uniform X {\n#define ORB_CONSTANTS_END };\n#define ORB_CONSTANT(T, N) T N\n";
				else
					constantsMacrosString = "#define ORB_CONSTANTS_BEGIN(X)\n#define ORB_CONSTANTS_END\n#define ORB_CONSTANT(T, N) uniform T N\n";

				/* 'varying' and 'attribute' was replaced with 'in' and 'out' in GLES 3 and GL 3.3 */
				std::string_view varyingString;
				std::string_view attributeString;
				if( ( implCtx->embedded && implCtx->version >= version( 3 ) ) || ( implCtx->version >= version( 3, 3 ) ) )
				{
					varyingString   = "#define ORB_VARYING out\n";
					attributeString = "#define ORB_ATTRIBUTE in\n";
				}
				else
				{
					varyingString   = "#define ORB_VARYING varying\n";
					attributeString = "#define ORB_ATTRIBUTE attribute\n";
				}

				const std::string_view glslDefineString = "#define ORB_GLSL 1\n";
				const auto&            data             = ast.get_data();

				const GLchar* sources[] =
				{
					versionString.data(),
					glslDefineString.data(),
					constantsMacrosString.data(),
					varyingString.data(),
					attributeString.data(),
					reinterpret_cast< const GLchar* >( data.data() ),
				};
				const GLint lengths[] =
				{
					static_cast< GLint >( versionString.size() ),
					static_cast< GLint >( glslDefineString.size() ),
					static_cast< GLint >( constantsMacrosString.size() ),
					static_cast< GLint >( varyingString.size() ),
					static_cast< GLint >( attributeString.size() ),
					static_cast< GLint >( data.size() ),
				};

				functions.shader_source( impl->id, count_of( sources ), sources, lengths );
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
				auto                   impl     = std::addressof( m_impl.emplace< __vertex_shader_impl_d3d11 >() );
				const auto&            data     = ast.get_data();
				const D3D_SHADER_MACRO macros[] = { { "ORB_HLSL", "1" }, { NULL, NULL } };
				UINT                   flags    = ( D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS );
			#if defined( _DEBUG )
				flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION );
			#else
				flags |= ( D3DCOMPILE_OPTIMIZATION_LEVEL3 );
			#endif

				ID3DBlob* vertexData   = nullptr;
				ID3DBlob* vertexErrors = nullptr;
				if( D3DCompile( data.data(), data.size(), NULL, macros, nullptr, "main", "vs_5_0", flags, 0, &vertexData, &vertexErrors ) != S_OK )
				{
					log_error( format( "%s", static_cast< const char* >( vertexErrors->GetBufferPointer() ) ) );
					vertexErrors->Release();
					return;
				}

				ID3D11Device&       device = *( std::get_if< __render_context_impl_d3d11 >( currentContextImpl )->device );
				ID3D11VertexShader* vertexShader;
				device.CreateVertexShader( vertexData->GetBufferPointer(), vertexData->GetBufferSize(), nullptr, &vertexShader );
				impl->vertexData.reset( vertexData );
				impl->vertexShader.reset( vertexShader );

				break;
			}
		#endif
		}
	}
}
