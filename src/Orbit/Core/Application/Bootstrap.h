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
#include "Orbit/Core/Core.h"

#include <memory>

ORB_NAMESPACE_BEGIN

namespace Bootstrap
{
	using Trampoline = std::shared_ptr< void >( * )( void );

	extern ORB_API_CORE Trampoline trampoline;

	template< typename T >
#if defined( ORB_CC_CLANG ) || defined( ORB_CC_GCC )
	__attribute__(( constructor ))
#endif // ORB_CC_CLANG || defined( ORB_CC_GCC )
	auto Load( void )
	{
		trampoline = []( void ){ return std::static_pointer_cast< void >( std::make_shared< T >() ); };
		return 0;
	}
}

template< typename T >
class Bootstrapper
{
private:

#if defined( ORB_CC_MSVC )
	static inline auto bootstrap_eval = Bootstrap::Load< T >();
#elif defined( ORB_CC_CLANG ) || defined( ORB_CC_GCC ) // ORB_CC_MSVC
	using BootstrapDecl = decltype( Bootstrap::Load< T >() );
#endif // ORB_CC_CLANG || ORB_CC_GCC

};

ORB_NAMESPACE_END
