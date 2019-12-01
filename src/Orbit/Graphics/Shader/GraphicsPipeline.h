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

#pragma once
#include "Orbit/Graphics/Impl/GraphicsPipelineImpl.h"

ORB_NAMESPACE_BEGIN

class FragmentShader;
class IndexBuffer;
class VertexBuffer;
class VertexShader;

class ORB_API_GRAPHICS GraphicsPipeline
{
public:

	 GraphicsPipeline( void );
	~GraphicsPipeline( void );

public:

	void Bind                ( void );
	void Unbind              ( void );
	void SetShaders          ( const VertexShader& vert, const FragmentShader& frag );
	void DescribeVertexLayout( VertexLayout layout );
	void Draw                ( const VertexBuffer& vb );
	void Draw                ( const IndexBuffer& ib );

private:

	Private::GraphicsPipelineImpl m_impl;

};

ORB_NAMESPACE_END
