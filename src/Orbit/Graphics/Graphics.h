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

#pragma once
#include <cstdint>

#include "Orbit/Core/Utility/Bitmask.h"

#if defined( ORB_BUILD_GRAPHICS )
#  define ORB_API_GRAPHICS ORB_DLL_EXPORT
#else
#  define ORB_API_GRAPHICS ORB_DLL_IMPORT
#endif

/* Graphics API macros. */
#if defined( ORB_OS_WINDOWS )
#  define ORB_HAS_D3D11  1
#  define ORB_HAS_OPENGL 1
#elif defined( ORB_OS_LINUX )
#  define ORB_HAS_D3D11  0
#  define ORB_HAS_OPENGL 1
#elif defined( ORB_OS_MACOS )
#  define ORB_HAS_D3D11  0
#  define ORB_HAS_OPENGL 1
#elif defined( ORB_OS_ANDROID )
#  define ORB_HAS_D3D11  0
#  define ORB_HAS_OPENGL 1
#elif defined( ORB_OS_IOS )
#  define ORB_HAS_D3D11  0
#  define ORB_HAS_OPENGL 1
#endif

/* Direct3D includes */
#if( ORB_HAS_D3D11 )
#  include <d3d11.h>
#endif

/* OpenGL includes */
#if( ORB_HAS_OPENGL )
#  if defined( ORB_OS_WINDOWS )
#    include <Windows.h>
#    include <gl/GL.h>
#  elif defined( ORB_OS_LINUX )
#    include <GL/glx.h>
#    include <GL/gl.h>
#  elif defined( ORB_OS_MACOS )
#    include <OpenGL/gl.h>
#  elif defined( ORB_OS_ANDROID )
#    include <EGL/egl.h>
#    include <EGL/eglext.h>
#    include <GLES3/gl3.h>
#  elif defined( ORB_OS_IOS )
#    include <OpenGLES/ES3/gl.h>
#  endif
#endif

ORB_NAMESPACE_BEGIN

enum class GraphicsAPI
{
	Null = 0,
	D3D11,
	OpenGL,
};

#if( ORB_HAS_D3D11 )
constexpr GraphicsAPI default_graphics_api = GraphicsAPI::D3D11;
#elif( ORB_HAS_OPENGL )
constexpr GraphicsAPI default_graphics_api = GraphicsAPI::OpenGL;
#else
constexpr GraphicsAPI default_graphics_api = GraphicsAPI::Null;
#endif

/* Enumerators */

enum class BufferMask
{
	Color = 0x1,
	Depth = 0x2,
};
ORB_ENABLE_BITMASKING( BufferMask );

enum class ShaderType
{
	Fragment = 1,
	Vertex   = 2,
};

enum class IndexFormat
{
	Byte,
	Word,
	DoubleWord,
};

template< typename T >
struct IndexFormatTraits
{
	static constexpr bool        enabled = false;
	static constexpr IndexFormat format  = static_cast< IndexFormat >( 0 );
};

template< typename T >
constexpr auto is_index_format_v = IndexFormatTraits< T >::enabled;

template< typename T >
constexpr auto index_format_v = IndexFormatTraits< T >::format;

template<>
struct IndexFormatTraits< uint8_t >
{
	static constexpr bool        enabled = true;
	static constexpr IndexFormat format  = IndexFormat::Byte;
};

template<>
struct IndexFormatTraits< uint16_t >
{
	static constexpr bool        enabled = true;
	static constexpr IndexFormat format  = IndexFormat::Word;
};

template<>
struct IndexFormatTraits< uint32_t >
{
	static constexpr bool        enabled = true;
	static constexpr IndexFormat format  = IndexFormat::DoubleWord;
};

ORB_NAMESPACE_END
