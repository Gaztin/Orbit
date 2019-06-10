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

#include <android/asset_manager.h>

#include "orbit/core/android_app.h"

namespace orb
{
	namespace platform
	{
		asset_handle_t open_asset( const std::string& path )
		{
			AAssetManager* mgr = android_only::app->activity->assetManager;
			return AAssetManager_open( mgr, path.c_str(), AASSET_MODE_BUFFER );
		}

		size_t get_asset_size( asset_handle_t handle )
		{
			const off64_t len = AAsset_getLength64( handle );
			if( len < 0 )
				return 0;

			return static_cast< size_t >( len );
		}

		size_t read_asset_data( asset_handle_t handle, void* buf, size_t size )
		{
			const int numBytesRead = AAsset_read( handle, buf, size );
			if( numBytesRead < 0 )
				return 0;

			return static_cast< size_t >( numBytesRead );
		}

		bool close_asset( asset_handle handle )
		{
			AAsset_close( handle );
			return true;
		}
	}
}
