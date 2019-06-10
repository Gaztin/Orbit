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

#include "asset_handle.h"

namespace orb
{
	namespace platform
	{
		asset_handle_t open_asset( const std::string& path )
		{
			HANDLE handle = CreateFileA( path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if( handle == INVALID_HANDLE_VALUE )
				return NULL;

			return { handle };
		}

		size_t get_asset_size( asset_handle_t handle )
		{
			LARGE_INTEGER fileSize = { };
			if( GetFileSizeEx( handle, &fileSize ) == 0 )
				return 0;

			return static_cast< size_t >( fileSize.QuadPart );
		}

		size_t read_asset_data( asset_handle_t handle, void* buf, size_t size )
		{
			DWORD numBytesRead;
			if( !ReadFile( handle, buf, static_cast< DWORD >( size ), &numBytesRead, NULL ) )
				return 0;

			return static_cast< size_t >( numBytesRead );
		}

		bool close_asset( asset_handle_t handle )
		{
			if( handle == NULL || handle == INVALID_HANDLE_VALUE )
				return false;

			return CloseHandle( handle );
		}
	}
}
