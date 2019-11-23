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

#include "gl_version.h"

namespace orb
{
	namespace gl
	{
		extern ORB_API_GRAPHICS version get_system_default_opengl_version()
		{
		#if defined( ORB_OS_WINDOWS )
			return version::v4_1;
		#elif defined( ORB_OS_LINUX )
			return version::v4_1;
		#elif defined( ORB_OS_MACOS )
			return version::v2_0;
		#elif defined( ORB_OS_ANDROID )
			return version::vES_3;
		#elif defined( ORB_OS_IOS )
			return version::vES_2;
		#endif
		}
	}
}

