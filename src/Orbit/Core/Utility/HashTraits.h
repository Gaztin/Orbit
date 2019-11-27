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
#include <cstdint>

#include "Orbit/Core/Core.h"

ORB_NAMESPACE_BEGIN

/* FNV constants (reference: https://tools.ietf.org/html/draft-eastlake-fnv-17) */

template< uint32_t HashSize >
struct HashTraitsFNA;

template<>
struct HashTraitsFNA< 4u >
{
	using ValueType = uint32_t;

	static constexpr ValueType prime        =   16777619u;
	static constexpr ValueType offset_basis = 2166136261u;
};

template<>
struct HashTraitsFNA< 8u >
{
	using ValueType = uint64_t;

	static constexpr ValueType prime        =        1099511628211ull;
	static constexpr ValueType offset_basis = 14695981039346656037ull;
};

ORB_NAMESPACE_END
