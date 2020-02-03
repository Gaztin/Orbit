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

#pragma once
#include "Orbit/Graphics/Graphics.h"

ORB_NAMESPACE_BEGIN

class Vector3;
class Vector4;

class ORB_API_GRAPHICS IModelLoader
{
public:

	IModelLoader( void ) = default;
	virtual ~IModelLoader( void ) = default;

public:

	virtual bool   Init                  ( ByteSpan data ) = 0;
	virtual size_t PeekVertexCount       ( void )          = 0;
	virtual size_t PeekFaceCount         ( void )          = 0;
	virtual size_t PeekMeshCount         ( void )          = 0;
	virtual void   ReadNextVertexPosition( Vector4* out )  = 0;
	virtual void   ReadNextVertexNormal  ( Vector3* out )  = 0;
	virtual void   ReadNextFace          ( size_t* out )   = 0;
	virtual bool   ShouldGenerateNormals ( void )          = 0;

};

ORB_NAMESPACE_END
