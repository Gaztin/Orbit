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
#include "Orbit/Core/Utility/Utility.h"
#include "Orbit/Graphics/Private/ConstantBufferDetails.h"

#include <memory>

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS ConstantBuffer
{
public:

	explicit ConstantBuffer( size_t size );
	        ~ConstantBuffer( void );

public:

	void Bind  ( ShaderType type, uint32_t local_slot, uint32_t global_slot ) const;
	void Unbind( ShaderType type, uint32_t local_slot, uint32_t global_slot ) const;

public:

	void Update( const void* data, size_t size );

private:

	void* UpdateBegin( size_t size );
	void  UpdateEnd  ( void );

private:

	Private::ConstantBufferDetails details_;

};

ORB_NAMESPACE_END
