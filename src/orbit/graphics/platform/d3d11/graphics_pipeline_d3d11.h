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
#include "orbit/core/memory.h"
#include "orbit/graphics/platform/d3d11/d3d11.h"
#include "orbit/graphics/platform/graphics_pipeline_base.h"

namespace orb
{
namespace platform
{

class ORB_API_GRAPHICS graphics_pipeline_d3d11 : public graphics_pipeline_base
{
public:
	graphics_pipeline_d3d11();

	void add_shader(const shader& shr) final override;
	void describe_vertex_layout(vertex_layout layout) final override;

	void draw(size_t vertexCount) final override;

private:
	ID3DBlob* m_vertexData;
	ID3D11PixelShader* m_pixelShader;
	ID3D11VertexShader* m_vertexShader;
	com_ptr<ID3D11InputLayout> m_inputLayout;
};

}
}
