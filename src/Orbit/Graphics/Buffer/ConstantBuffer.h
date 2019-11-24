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
	ConstantBuffer( const std::tuple< Types... >& constants )
		: ConstantBuffer( ( 0 + ... + sizeof( Types ) ) )
	{
		Update( constants );
	}

	~ConstantBuffer();

	void Bind   ( ShaderType type, uint32_t slot );
	void Update ( void* dst, size_t location, const void* data, size_t size );

	template< typename... Types >
	void Update( const std::tuple< Types... >& constants )
	{
		void* dst = UpdateBegin( ( sizeof( Types ) + ... ) );
		UpdateSequencial( dst, constants, MakeSequence< sizeof...( Types ) >{ } );
		UpdateEnd();
	}

private:
	void* UpdateBegin ( size_t size );
	void  UpdateEnd   ( void );

	template< typename Tup, size_t... Is >
	void UpdateSequencial( void* dst, const Tup& tup, Sequence< Is... > )
	{
		[[ maybe_unused ]] auto l = { ( Update( reinterpret_cast< uint8_t* >( dst ) + std::distance( &std::get< 0 >( tup ), &std::get< Is >( tup ) ), Is, &std::get< Is >( tup ), sizeof( std::get< Is >( tup ) ) ), 0 )... };
	}

	ConstantBufferImpl m_impl;

};

ORB_NAMESPACE_END
