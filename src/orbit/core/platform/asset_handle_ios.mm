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

#include <Foundation/Foundation.h>

namespace orb
{
	namespace platform
	{
		asset_handle_t open_asset( const std::string& path )
		{
			NSString* nsPath   = [ NSString stringWithUTF8String:path.c_str() ];
			NSString* resource = [ nsPath stringByDeletingPathExtension ];
			NSString* type     = [ nsPath pathExtension ];
			NSBundle* bundle   = [ NSBundle mainBundle ];
			NSString* resPath  = [ bundle pathForResource:resource ofType:type inDirectory:@"assets" ];

			asset_handle_t handle = open( [ resPath UTF8String ], O_RDONLY );
			if( handle < 0 )
				return 0;

			return handle;
		}

		size_t get_asset_size( asset_handle handle )
		{
			lseek( handle, 0, SEEK_SET );
			const off_t off = lseek( handle, 0, SEEK_END );
			lseek( handle, 0, SEEK_SET );

			if( off < 0 )
				return 0;

			return static_cast< size_t >( off );
		}

		size_t read_asset_data( asset_handle handle, void* buf, size_t size )
		{
			const ssize_t numBytesRead = read( handle, buf, size );
			if( numBytesRead < 0 )
				return 0;

			return static_cast< size_t >( numBytesRead );
		}

		bool close_asset( asset_handle handle )
		{
			if( handle <= 0 )
				return false;

			return ( close( ah ) == 0 );
		}
	}
}
