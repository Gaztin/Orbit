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

#include "asset.h"

#include <algorithm>

#if defined( ORB_OS_WINDOWS )
#include <Windows.h>
#elif defined( ORB_OS_ANDROID )
#include <android/asset_manager.h>
#endif

namespace orb
{
	asset::asset( std::string_view path )
	{

	#if defined( ORB_OS_WINDOWS )

		HANDLE handle = CreateFileA( path.data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( handle != INVALID_HANDLE_VALUE )
		{
			do
			{
				LARGE_INTEGER fileSize;
				if( GetFileSizeEx( handle, &fileSize ) == 0 )
					break;

				m_data.resize( static_cast< size_t >( fileSize.QuadPart ) );
				ReadFile( handle, &m_data[ 0 ], m_data.size(), NULL, NULL );

			} while( false );

			CloseHandle( handle );
		}

	#elif defined( ORB_OS_LINUX ) || defined( ORB_OS_MACOS )

		int fd = open( path.data(), O_RDONLY );
		if( fd >= 0 )
		{
			do
			{
				lseek( fd, 0, SEEK_SET );
				const off_t len = lseek( fd, 0, SEEK_END );
				lseek( fd, 0, SEEK_SET );
				if( fileSize < 0 )
					break;

				m_data.resize( static_cast< size_t >( len ) );
				read( fd, &m_data[ 0 ], m_data.size() );

			} while( false );

			close( fd );
		}

	#elif defined( ORB_OS_ANDROID )

		AAsset* aAsset = AAssetManager_open( android_only::app->activity->assetManager, path.data(), AASSET_MODE_BUFFER );
		if( aAsset != nullptr )
		{
			do
			{
				const off64_t len = AAsset_getLength64( aAsset );
				if( len < 0 )
					break;

				m_data.resize( static_cast< size_t >( len ) );
				AAsset_read( aAsset, &m_data[ 0 ], m_data.size() );

			} while( false );

			AAsset_close( aAsset );
		}

	#elif defined( ORB_OS_IOS )

		NSString* nsPath         = [ NSString stringWithUTF8String : path.data() ];
		NSString* nsBaseName     = [ nsPath stringByDeletingPathExtension ];
		NSString* nsExtension    = [ nsPath pathExtension ];
		NSString* nsResourcePath = [ [ NSBundle mainBundle ] pathForResource : resource ofType : type inDirectory : @"assets" ];

		int fd = open( [ resPath UTF8String ], O_RDONLY );
		if( fd >= 0 )
		{
			do
			{
				lseek( fd, 0, SEEK_SET );
				const off_t len = lseek( fd, 0, SEEK_END );
				lseek( fd, 0, SEEK_SET );
				if( len < 0 )
					break;

				m_data.resize( static_cast< size_t >( len ) );
				read( fd, &m_data[ 0 ], m_data.size() );

			} while( false );

			close( fd );
		}

	#endif

	}
}
