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

/* Namespace macros */
#define ORB_NAMESPACE ::Orbit::
#define ORB_NAMESPACE_BEGIN namespace Orbit {
#define ORB_NAMESPACE_END }
#define ORB_NAMESPACED_OBJC( X ) Orbit##X

/* Utility macros */
#define ORB_DISABLE_COPY( CLASS )              \
	CLASS( const CLASS& )            = delete; \
	CLASS& operator=( const CLASS& ) = delete
#define ORB_DISABLE_MOVE( CLASS )         \
	CLASS( CLASS&& )            = delete; \
	CLASS& operator=( CLASS&& ) = delete
#define ORB_DISABLE_COPY_AND_MOVE( CLASS )     \
	CLASS( const CLASS& )            = delete; \
	CLASS( CLASS&& )                 = delete; \
	CLASS& operator=( const CLASS& ) = delete; \
	CLASS& operator=( CLASS&& )      = delete

/* Per-compiler macros. */
#if defined( _MSC_VER )
#  define ORB_CC_MSVC 1
#  define ORB_DLL_EXPORT __declspec( dllexport )
#  define ORB_DLL_IMPORT __declspec( dllimport )
#  define ORB_DLL_LOCAL
#elif defined( __clang__ ) // _MSC_VER
#  define ORB_CC_CLANG 1
#  define ORB_DLL_EXPORT __attribute__( ( visibility( "default" ) ) )
#  define ORB_DLL_IMPORT __attribute__( ( visibility( "default" ) ) )
#  define ORB_DLL_LOCAL  __attribute__( ( visibility( "hidden" ) ) )
#elif defined( __GNUC__ ) // __clang__
#  define ORB_CC_GCC 1
#  define ORB_DLL_EXPORT __attribute__( ( visibility( "default" ) ) )
#  define ORB_DLL_IMPORT __attribute__( ( visibility( "default" ) ) )
#  define ORB_DLL_LOCAL  __attribute__( ( visibility( "hidden" ) ) )
#endif // __GNUC__

/* Per-system macros. */
#if defined( _WIN32 )
#  define ORB_OS_WINDOWS 1
#elif defined( __ANDROID__ ) // _WIN32
#  define ORB_OS_ANDROID 1
#elif defined( __linux__ ) // __ANDROID__
#  define ORB_OS_LINUX 1
#elif defined( __APPLE__ ) //  __linux__
#  include <TargetConditionals.h>
#  if( TARGET_OS_IPHONE )
#    define ORB_OS_IOS 1
#  elif( TARGET_OS_MAC ) // TARGET_OS_IPHONE
#    define ORB_OS_MACOS 1
#  endif // TARGET_OS_MAC
#endif // __APPLE__

#if defined( ORB_CC_MSVC )
/* Suppress MSVC warnings about DLL-interfaces */
#  pragma warning( disable: 4251 )
#endif // ORB_CC_MSVC
