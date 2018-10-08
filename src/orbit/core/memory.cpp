/*
* Copyright (c) 2018 Sebastian Kylander http://gaztin.com/
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

#include "memory.h"

#if defined(ORB_OS_ANDROID)

/* The Android NDK provides no means for aligned new and delete, as further supported in this issue:
 * https://github.com/android-ndk/ndk/issues/421. To fix this, we implement the necessary operators ourselves.
 */

#include <cstdlib>
#include <new>

namespace std
{
enum class align_val_t : size_t {};
}

void* operator new(size_t size, std::align_val_t alignment) noexcept(false)
{
	void* ptr;
	if (posix_memalign(&ptr, static_cast<size_t>(alignment), size) != 0)
		throw std::bad_alloc();
	else
		return ptr;
}

void operator delete(void* ptr, std::align_val_t) noexcept
{
	free(ptr);
}

#endif
