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
#include "Orbit/Core/Utility/Span.h"
#include "Orbit/Graphics/Model/Loader/IModelLoader.h"

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS OBJLoader final : public IModelLoader
{
public:

	bool   Init                  ( ByteSpan data ) override;
	size_t PeekVertexCount       ( void )          override;
	size_t PeekFaceCount         ( void )          override;
	size_t PeekMeshCount         ( void )          override;
	void   ReadNextVertexPosition( Vector4* out )  override;
	void   ReadNextVertexNormal  ( Vector3* out )  override;
	void   ReadNextFace          ( size_t* out )   override;
	bool   ShouldGenerateNormals ( void )          override;

private:

	ByteSpan m_data;

	const uint8_t* m_vertex_location;
	const uint8_t* m_face_location;

};

ORB_NAMESPACE_END
