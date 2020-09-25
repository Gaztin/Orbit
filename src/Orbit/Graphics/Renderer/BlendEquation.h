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
#include "Orbit/Core/Color/RGBA.h"
#include "Orbit/Graphics/Graphics.h"

ORB_NAMESPACE_BEGIN

class ORB_API_GRAPHICS BlendEquation
{
public:

	constexpr BlendEquation( void )
		: src_factor_color( BlendFactor::One )
		, src_factor_alpha( BlendFactor::One )
		, dst_factor_color( BlendFactor::Zero )
		, dst_factor_alpha( BlendFactor::Zero )
		, op_color        ( BlendOp::Add )
		, op_alpha        ( BlendOp::Add )
	{
	}

	constexpr BlendEquation( BlendFactor src_factor, BlendFactor dst_factor, BlendOp op )
		: src_factor_color( src_factor )
		, src_factor_alpha( src_factor )
		, dst_factor_color( dst_factor )
		, dst_factor_alpha( dst_factor )
		, op_color        ( op )
		, op_alpha        ( op )
	{
	}

public:

	/** Enable this class to be sortable */
	constexpr bool operator<( const BlendEquation& rhs ) const
	{
		return ( ( src_factor_color < rhs.src_factor_color ) ||
		         ( src_factor_alpha < rhs.src_factor_alpha ) ||
		         ( dst_factor_color < rhs.dst_factor_color ) ||
		         ( dst_factor_alpha < rhs.dst_factor_alpha ) ||
		         ( op_color < rhs.op_color )                 ||
		         ( op_alpha < rhs.op_alpha )                 ||
		         ( constant.r < rhs.constant.r )             ||
		         ( constant.g < rhs.constant.g )             ||
		         ( constant.b < rhs.constant.b )             ||
		         ( constant.a < rhs.constant.a )
		);
	}

public:

	BlendFactor src_factor_color;
	BlendFactor src_factor_alpha;
	BlendFactor dst_factor_color;
	BlendFactor dst_factor_alpha;
	BlendOp     op_color;
	BlendOp     op_alpha;
	RGBA        constant;

};

constexpr BlendEquation operator+( BlendFactor a, BlendFactor b )
{
	return BlendEquation( a, b, BlendOp::Add );
}

constexpr BlendEquation operator-( BlendFactor a, BlendFactor b )
{
	return BlendEquation( a, b, BlendOp::Subtract );
}

ORB_NAMESPACE_END
