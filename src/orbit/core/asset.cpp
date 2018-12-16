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

#include "asset.h"

#include <algorithm>

#if defined(ORB_OS_WINDOWS)
#include <windows.h>
#endif

namespace orb
{

asset::asset(const char* file_name)
{
#if defined(ORB_OS_WINDOWS)

	struct file
	{
		~file() { if (handle) CloseHandle(handle); }
		HANDLE handle;
	};

	file f{};
	f.handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f.handle == INVALID_HANDLE_VALUE)
	{
		f.handle = NULL;
		return;
	}

	LARGE_INTEGER fileSize{};
	if (GetFileSizeEx(f.handle, &fileSize) == 0)
		return;

	m_data.resize(static_cast<size_t>(fileSize.QuadPart));

	DWORD numBytesRead;
	ReadFile(f.handle, m_data.data(), static_cast<DWORD>(fileSize.QuadPart), &numBytesRead, NULL);

#endif
}

}
