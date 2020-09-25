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

#include "Animation.h"

#include "Orbit/Core/IO/File/Markup/XML/XMLFile.h"
#include "Orbit/Core/IO/Log.h"
#include "Orbit/Math/Matrix/Matrix4.h"

#include <algorithm>
#include <cstddef>
#include <sstream>

ORB_NAMESPACE_BEGIN

Animation::Animation( JointKeyFrameMap keyframes )
	: keyframes_( std::move( keyframes ) )
{
	// Figure out duration by finding the last keyframe
	for( auto& it : keyframes_ )
	{
		for( const KeyFrame& keyframe : it.second )
		{
			if( keyframe.time > duration_ )
				duration_ = keyframe.time;
		}
	}
}

Matrix4 Animation::JointPoseAtTime( std::string_view joint, float time ) const
{
	if( auto it = keyframes_.find( std::string( joint ) ); it != keyframes_.end() )
	{
		for( const KeyFrame& kf : it->second )
		{
			if( kf.time >= time )
			{
				// TODO: Interpolate poses
				return kf.transform;
			}
		}

		if( !it->second.empty() )
			return it->second.back().transform;
	}

	return Matrix4();
}

ORB_NAMESPACE_END
