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
#include "Orbit/Core/Platform/Windows/ComPtr.h"
#include "Orbit/Graphics/Graphics.h"

#include <variant>

ORB_NAMESPACE_BEGIN

namespace Private
{

#if( ORB_HAS_OPENGL )

	struct _Texture2DDetailsOpenGL
	{
		GLuint id;
	};

#endif // ORB_HAS_OPENGL
#if( ORB_HAS_D3D11 )

	struct _Texture2DDetailsD3D11
	{
		ComPtr< ID3D11Texture2D >          texture2d;
		ComPtr< ID3D11ShaderResourceView > shader_resource_view;
	};

#endif // ORB_HAS_D3D11

	using Texture2DDetails = std::variant< std::monostate
	#if( ORB_HAS_OPENGL )
		, _Texture2DDetailsOpenGL
	#endif // ORB_HAS_OPENGL
	#if( ORB_HAS_D3D11 )
		, _Texture2DDetailsD3D11
	#endif // ORB_HAS_D3D11
	>;
}

ORB_NAMESPACE_END
