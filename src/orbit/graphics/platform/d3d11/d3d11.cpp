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

#include "d3d11.h"

#include "orbit/graphics/platform/d3d11/render_context_d3d11.h"
#include "orbit/graphics/render_context.h"

namespace orb
{
	namespace d3d11
	{
		com_ptr< ID3D11Buffer > create_buffer( bind_flag bf, const void* data, size_t size, usage usg, cpu_access cpu )
		{
			ID3D11Device& device = static_cast< platform::render_context_d3d11& >( render_context::get_current()->get_base() ).get_device();

			D3D11_BUFFER_DESC desc = { };
			desc.ByteWidth      = static_cast< UINT >( ( size + 0xf ) & ~0xf ); /* Align by 16 bytes */
			desc.Usage          = static_cast< D3D11_USAGE >( usg );
			desc.BindFlags      = static_cast< D3D11_BIND_FLAG >( bf );
			desc.CPUAccessFlags = static_cast< D3D11_CPU_ACCESS_FLAG >( cpu );

			ID3D11Buffer* buffer = nullptr;
			if( data )
			{
				D3D11_SUBRESOURCE_DATA initialData = { };
				initialData.pSysMem = data;
				device.CreateBuffer( &desc, &initialData, &buffer );
			}
			else
			{
				device.CreateBuffer( &desc, nullptr, &buffer );
			}

			return com_ptr< ID3D11Buffer >( buffer );
		}
	}
}
