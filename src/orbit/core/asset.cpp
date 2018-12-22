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

#include "asset.h"

#include <algorithm>

#include "orbit/core/platform/asset_handle.h"

namespace orb
{

asset::asset(const std::string& path)
{
	platform::asset_handle ah{};
	ah = platform::open_asset(path);
	if (!ah)
		return;

	const size_t sz = platform::get_asset_size(ah);
	if (sz > 0)
	{
		m_data.resize(sz);
		platform::read_asset_data(ah, m_data.data(), m_data.size());
	}

	platform::close_asset(ah);
}

}
