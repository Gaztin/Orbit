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

#include "Orbit/Core/IO/Parser/XML/XMLParser.h"
#include "Orbit/Core/IO/Log.h"
#include "Orbit/Math/Matrix4.h"

#include <cstddef>
#include <sstream>

ORB_NAMESPACE_BEGIN

static bool SortKeyFrames( const KeyFrame& a, const KeyFrame& b )
{
	return ( a.time < b.time );
}

Animation::Animation( ByteSpan data )
	: m_duration( 0.0 )
{
	if( !ParseCollada( data ) )
	{
		LogErrorString( "Failed to load animation. Unrecognized format." );
	}
}

Matrix4 Animation::JointPoseAtTime( std::string_view joint, float time ) const
{
	if( auto it = m_joint_key_frames.find( std::string( joint ) ); it != m_joint_key_frames.end() )
	{
		for( const KeyFrame& kf : it->second )
		{
			if( kf.time >= time )
			{
				/* TODO: Interpolate poses */
				return kf.transform;
			}
		}

		if( !it->second.empty() )
			return it->second.back().transform;
	}

	return Matrix4();
}

bool Animation::ParseCollada( ByteSpan data )
{
	XMLParser parser( data );

	const XMLElement& collada            = parser.GetRootElement()[ "COLLADA" ];
	const XMLElement& library_animations = collada[ "library_animations" ];

	for( const XMLElement& animation : library_animations )
	{
		if( animation.name != "animation" )
			continue;

		const XMLElement& sampler = animation[ "sampler" ];

		std::string input_source_id;
		{
			std::istringstream ss( std::string( sampler.ChildWithAttribute( "input", "semantic", "INPUT" ).Attribute( "source" ) ) );
			ss.ignore( 1 );
			ss >> input_source_id;
		}

		std::string output_source_id;
		{
			std::istringstream ss( std::string( sampler.ChildWithAttribute( "input", "semantic", "OUTPUT" ).Attribute( "source" ) ) );
			ss.ignore( 1 );
			ss >> output_source_id;
		}

		std::string interpolation_source_id;
		{
			std::istringstream ss( std::string( sampler.ChildWithAttribute( "input", "semantic", "INTERPOLATION" ).Attribute( "source" ) ) );
			ss.ignore( 1 );
			ss >> interpolation_source_id;
		}

		std::string target_joint;
		{
			std::istringstream ss( std::string( animation[ "channel" ].Attribute( "target" ) ) );
			ss >> target_joint;
			target_joint.erase( target_joint.rfind( "/" ) );
		}

		std::vector< KeyFrame > key_frames;
		for( const XMLElement& source : animation )
		{
			if( source.name != "source" )
				continue;

			if( key_frames.empty() )
			{
				size_t count = 0;
				{
					std::istringstream ss( std::string( source[ "technique_common" ][ "accessor" ].Attribute( "count" ) ) );
					ss >> count;
				}

				key_frames.resize( count );
			}

			const std::string source_id( source.Attribute( "id" ) );

			if( source_id == input_source_id )
			{
				std::istringstream ss( source[ "float_array" ].content );

				for( KeyFrame& kf : key_frames )
					ss >> kf.time;
			}
			else if( source_id == output_source_id )
			{
				std::istringstream ss( source[ "float_array" ].content );

				for( KeyFrame& kf : key_frames )
				{
					for( size_t e = 0; e < 16; ++e )
						ss >> kf.transform[ e ];

//					kf.transform.Transpose();
				}
			}
			else if( source_id == interpolation_source_id )
			{
				std::istringstream ss( source[ "Name_array" ].content );

				/* One of: LINEAR, BEZIER, CARDINAL, HERMITE, BSPLINE and STEP */
				for( KeyFrame& kf : key_frames )
					ss >> kf.interpolation_type;
			}
		}

		std::sort( key_frames.begin(), key_frames.end(), SortKeyFrames );

		if( !key_frames.empty() )
		{
			const KeyFrame& last_frame = key_frames.back();

			if( last_frame.time > m_duration )
				m_duration = last_frame.time;
		}

		m_joint_key_frames.try_emplace( std::move( target_joint ), std::move( key_frames ) );
	}

	return true;
}

ORB_NAMESPACE_END
