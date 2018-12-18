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
#elif defined(ORB_OS_ANDROID)
#include "orbit/core/android_app.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#elif defined(ORB_OS_LINUX) || defined(ORB_OS_MACOS)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace orb
{

asset::asset(const char* file_path)
{
#if defined(ORB_OS_WINDOWS)

	struct file
	{
		~file() { if (handle) CloseHandle(handle); }
		HANDLE handle;
	};

	file f{};
	f.handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
	ReadFile(f.handle, m_data.data(), static_cast<DWORD>(m_data.size()), &numBytesRead, NULL);

#elif defined(ORB_OS_ANDROID)

	struct aasset
	{
		~aasset() { if (ast) AAsset_close(ast); }
		AAsset* ast;
	};

	AAssetManager* mgr = android_only::app->activity->assetManager;
	aasset a{};
	a.ast = AAssetManager_open(mgr, file_path, AASSET_MODE_BUFFER);
	if (!a.ast)
		return;

	const off64_t len = AAsset_getLength64(a.ast);
	if (len <= 0)
		return;

	m_data.resize(static_cast<size_t>(len));
	AAsset_read(a.ast, m_data.data(), m_data.size());

#elif defined(ORB_OS_LINUX) || defined(ORB_OS_MACOS)

	struct file
	{
		~file() { if (fd > 0) close(fd); }
		int fd;
	};

	file f{};
	f.fd = open(file_path, O_RDONLY);
	if (f.fd < 0)
		return;

	const off_t sz = lseek(f.fd, 0, SEEK_END);
	if (sz <= 0)
		return;

	lseek(f.fd, 0, SEEK_SET);
	m_data.resize(static_cast<size_t>(sz));
	read(f.fd, m_data.data(), m_data.size());

#endif
}

}
