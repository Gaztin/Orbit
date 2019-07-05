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

#pragma once

#include <memory>
#include <type_traits>

#include "orbit/core.h"

#define ORB_APP_DECL( APP_TYPE )                                                                 \
    class APP_TYPE; /* Forward declaration */                                                    \
    volatile int __orb__app_initializer_eval = orb::application< APP_TYPE >::__initializer_eval; \
    class APP_TYPE final : public ::orb::application< APP_TYPE >

namespace orb
{
	class ORB_API_CORE application_base
	{
	public:
		application_base() = default;
		virtual ~application_base() = default;

		virtual void frame() { }
		virtual bool is_running() = 0;

		static void run_instance();

	protected:
		static std::shared_ptr< void >( *s_initializer )();
		static std::shared_ptr< application_base > s_instance;
	};

	template< typename Derived >
	class application : private application_base
	{
	public:
		static inline volatile auto __initializer_eval = [] { s_initializer = [] { return std::static_pointer_cast< void >( std::make_shared< Derived >() ); }; return 1; }();
	};
}
