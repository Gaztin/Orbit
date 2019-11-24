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
#include <memory>
#include <tuple>

#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/Impl/ConstantBufferImpl.h"

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS ConstantBuffer
{
public:
	explicit ConstantBuffer( size_t size );

	template< typename... Types >
	ConstantBuffer( const std::tuple< Types... >& )
		: ConstantBuffer( ( 0 + ... + sizeof( Types ) ) )
	{
	}

	~ConstantBuffer();

	void Bind   ( ShaderType type, uint32_t slot );
	void Update ( size_t location, const void* data, size_t size );

	template< typename... Types >
	void Update( const std::tuple< Types... >& constants )
	{
		if constexpr( sizeof...( Types ) > 0 )
			UpdateSequencial( constants, MakeSequence< sizeof...( Types ) >() );
		else
			Update( 0, nullptr, 0 );
	}

private:
	template< typename Tup, size_t... Is >
	void UpdateSequencial( Tup&& tup, Sequence< Is... > )
	{
		[[ maybe_unused ]] auto l = { ( Update( Is, &std::get< Is >( tup ), sizeof( std::get< Is >( tup ) ) ), 0 )... };
	}

	ConstantBufferImpl m_impl;

};

ORB_NAMESPACE_END
