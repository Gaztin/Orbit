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

#include "Asset.h"

#include "Orbit/Core/Debug/Trace.h"
#include "Orbit/Core/Platform/Android/AndroidApp.h"

#include <algorithm>

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#elif defined( ORB_OS_LINUX ) || defined( ORB_OS_MACOS ) // ORB_OS_WINDOWS
#  include <fcntl.h>
#  include <unistd.h>
#elif defined( ORB_OS_ANDROID ) // ORB_OS_LINUX || ORB_OS_MACOS
#  include <android/asset_manager.h>
#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID
#  include <Foundation/Foundation.h>
#endif // ORB_OS_IOS

ORB_NAMESPACE_BEGIN

Asset::Asset( std::string_view path )
{
	ORB_TRACE( "Loading asset: %s", path.data() );

#if defined( ORB_OS_WINDOWS )

	HANDLE handle = CreateFileA( path.data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( handle != INVALID_HANDLE_VALUE )
	{
		do
		{
			LARGE_INTEGER fileSize;
			if( GetFileSizeEx( handle, &fileSize ) == 0 )
				break;

			data_.resize( static_cast< size_t >( fileSize.QuadPart ) );
			ReadFile( handle, &data_[ 0 ], static_cast< DWORD >( data_.size() ), NULL, NULL );

		} while( false );

		CloseHandle( handle );
	}

#elif defined( ORB_OS_LINUX ) || defined( ORB_OS_MACOS ) // ORB_OS_WINDOWS

	int fd = open( path.data(), O_RDONLY );
	if( fd >= 0 )
	{
		do
		{
			lseek( fd, 0, SEEK_SET );
			const off_t len = lseek( fd, 0, SEEK_END );
			lseek( fd, 0, SEEK_SET );
			if( len < 0 )
				break;

			data_.resize( static_cast< size_t >( len ) );
			read( fd, &data_[ 0 ], data_.size() );

		} while( false );

		close( fd );
	}

#elif defined( ORB_OS_ANDROID ) // ORB_OS_LINUX || ORB_OS_MACOS

	AAsset* asset = AAssetManager_open( AndroidOnly::app->activity->assetManager, path.data(), AASSET_MODE_BUFFER );
	if( asset != nullptr )
	{
		do
		{
			const off64_t len = AAsset_getLength64( asset );
			if( len < 0 )
				break;

			data_.resize( static_cast< size_t >( len ) );
			AAsset_read( asset, &data_[ 0 ], data_.size() );

		} while( false );

		AAsset_close( asset );
	}

#elif defined( ORB_OS_IOS ) // ORB_OS_ANDROID

	NSString* ns_path          = [ NSString stringWithUTF8String : path.data() ];
	NSString* ns_base_name     = [ ns_path stringByDeletingPathExtension ];
	NSString* ns_extension     = [ ns_path pathExtension ];
	NSString* ns_resource_path = [ [ NSBundle mainBundle ] pathForResource:ns_base_name ofType:ns_extension inDirectory:@"assets" ];

	int fd = open( [ ns_resource_path UTF8String ], O_RDONLY );
	if( fd >= 0 )
	{
		do
		{
			lseek( fd, 0, SEEK_SET );
			const off_t len = lseek( fd, 0, SEEK_END );
			lseek( fd, 0, SEEK_SET );
			if( len < 0 )
				break;

			data_.resize( static_cast< size_t >( len ) );
			read( fd, &data_[ 0 ], data_.size() );

		} while( false );

		close( fd );
	}

#endif // ORB_OS_IOS

}

ORB_NAMESPACE_END
