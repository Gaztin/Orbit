/*
* Copyright (c) 2018 Sebastian Kylander http://gaztin.com/
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
#include <string>
#include <type_traits>
#include <utility>

#include "orbit/core.h"

namespace orb
{

template<typename T>
struct false_type : std::false_type { };

template<typename T>
struct in_place_type { };

template<typename T>
constexpr in_place_type<T> in_place_type_v { };

template<typename T, size_t c>
constexpr size_t count_of(T(&)[c])
{
	return c;
}

/* Integer sequences by courtesy of https://stackoverflow.com/a/16387374 */
template<size_t... Is>
struct seq { };

template<size_t N, size_t... Is>
struct gen_seq : gen_seq<(N - 1), (N - 1), Is...> { };

template<size_t... Is>
struct gen_seq<0, Is...> : seq<Is...> { };

template<typename Dst, typename Src>
Dst cast(Src src)
{
	/* Can implicitly convert. */
	if constexpr (std::is_convertible<Src, Dst>::value)
		return static_cast<Dst>(src);

	/* Can implicitly convert if const qualifier is removed from both. */
	else if constexpr (std::is_convertible<typename std::remove_const<Src>::type, typename std::remove_const<Dst>::type>::value)
		return const_cast<Dst>(src);

	/* Both are pointers. */
	else if constexpr (std::is_pointer<Src>::value && std::is_pointer<Dst>::value)
		return reinterpret_cast<Dst>(src);

	/* Both are references. */
	else if constexpr (std::is_reference<Src>::value && std::is_reference<Dst>::value)
		return reinterpret_cast<Dst>(src);

	/* Sizes match. */
	else if constexpr (sizeof(Src) == sizeof(Dst))
		return reinterpret_cast<Dst>(src);

	/* Invalid cast. */
	else static_assert(false_type<Src>::value, "Invalid cast");
}

template<typename... Args>
std::string format(const char* fmt, Args... args)
{
	const int len = snprintf(nullptr, 0, fmt, args...);
	if (len < 0)
		return std::string(fmt);

	std::string res(len, '\0');
	snprintf(&res[0], len + 1, fmt, args...);
	return res;
}

}
