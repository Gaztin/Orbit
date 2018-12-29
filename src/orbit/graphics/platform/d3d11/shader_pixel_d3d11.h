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

#pragma once
#include "orbit/core/asset.h"
#include "orbit/core/log.h"
#include "orbit/core/utility.h"
#include "orbit/graphics/platform/d3d11/d3d11.h"
#include "orbit/graphics/platform/shader_base.h"
#include "orbit/graphics/platform/d3d11/render_context_d3d11.h"
#include "orbit/graphics/render_context.h"

#if defined(ORB_OS_WINDOWS)
#include <d3dcompiler.h>
#endif

namespace orb
{
namespace platform
{

class ORB_API_GRAPHICS shader_pixel_d3d11 : public shader_base
{
public:
	shader_pixel_d3d11(const asset& ast)
	{
#if defined(ORB_OS_WINDOWS)
		const auto& data = ast.get_data();
		const D3D_SHADER_MACRO macros[] = { { "ORB_HLSL", "1" }, { NULL, NULL } };
		UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
		ID3DBlob* pixelData = nullptr;
		ID3DBlob* pixelErrors = nullptr;
		if (D3DCompile(data.data(), data.size(), NULL, macros, nullptr, "main", "ps_5_0", flags, 0, &pixelData, &pixelErrors) != S_OK)
		{
			log_error(format("%s", static_cast<const char*>(pixelErrors->GetBufferPointer())));
			pixelErrors->Release();
			return;
		}

		ID3D11Device& device = static_cast<render_context_d3d11&>(render_context::get_current()->get_base()).get_device();
		ID3D11PixelShader* pixelShader;
		device.CreatePixelShader(pixelData->GetBufferPointer(), pixelData->GetBufferSize(), nullptr, &pixelShader);
		m_pixelShader.reset(pixelShader);
		pixelData->Release();
#endif
	}

	shader_type get_type() const final override { return shader_type::Fragment; }

#if defined(ORB_OS_WINDOWS)
	const ID3D11PixelShader& get_pixel_shader() const { return *m_pixelShader; }
#endif

private:
#if defined(ORB_OS_WINDOWS)
	com_ptr<ID3D11PixelShader> m_pixelShader;
#endif
};

}
}
