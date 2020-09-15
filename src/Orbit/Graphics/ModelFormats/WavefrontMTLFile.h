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
#include "Orbit/Core/IO/File/TextFile.h"
#include "Orbit/Core/Color/RGB.h"
#include "Orbit/Graphics/Graphics.h"

#include <vector>

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS WavefrontMTLFile : public TextFile
{
public:

	enum class IlluminationModel
	{
		ColorOn_AmbientOff                                       = 0,
		ColorOn_AmbientOn                                        = 1,
		HighlightOn                                              = 2,
		ReflectionOn_RayTraceOn                                  = 3,
		TransparencyGlassOn_ReflectionRayTraceOn                 = 4,
		ReflectionFresnelOn_RayTraceOn                           = 5,
		TransparencyRefractionOn_ReflectionFresnelOff_RayTraceOn = 6,
		TransparencyRefractionOn_ReflectionFresnelOn_RayTraceOn  = 7,
		ReflectionOn_RayTraceOff                                 = 8,
		TransparencyGlassOn_ReflectionRayTraceOff                = 9,
		CastsShadowsOntoInvisibleSurfaces                        = 10,
	};

	struct Material
	{
		IlluminationModel illumination_model = IlluminationModel::ColorOn_AmbientOff;

		RGB ambient_color;
		RGB diffuse_color;
		RGB specular_color;

		float specular_exponent = 0.0f;
		float dissolved         = 1.0f;
		float optical_density   = 1.0f;
	};

public:

	explicit WavefrontMTLFile( ByteSpan data );

public:

	auto GetMaterials( void ) const { return materials_; }

private:

	std::vector< Material > materials_;

};

ORB_NAMESPACE_END
