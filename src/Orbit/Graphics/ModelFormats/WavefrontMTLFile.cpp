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

#include "WavefrontMTLFile.h"

#include <cstdio>
#include <optional>

ORB_NAMESPACE_BEGIN

WavefrontMTLFile::WavefrontMTLFile( ByteSpan data )
{
	Init( data.Size() );

	const char*               src = reinterpret_cast< const char* >( data.Ptr() );
	std::optional< Material > current_material;

	while( !IsEOF() )
	{
		const std::string_view line = ReadLine( src );

		// Skip empty lines
		if( line.empty() )
			continue;

		// Skip comments
		if( line[ 0 ] == '#' )
			continue;

//////////////////////////////////////////////////////////////////////////

		if( char new_material_name_buf[ 64 ]; std::sscanf( line.data(), "newtml %s", new_material_name_buf ) > 0 )
		{
			if( current_material )
				materials_.push_back( *current_material );

			current_material = std::make_optional< Material >();
		}
		else if( std::sscanf( line.data(), "Ns %f", &current_material->specular_exponent ) > 0 )
		{
		}
		else if( auto& Ka = current_material->ambient_color; std::sscanf( line.data(), "Ka %f %f %f", &Ka.r, &Ka.g, &Ka.b ) > 0 )
		{
		}
		else if( auto& Kd = current_material->diffuse_color; std::sscanf( line.data(), "Kd %f %f %f", &Kd.r, &Kd.g, &Kd.b ) > 0 )
		{
		}
		else if( auto& Ks = current_material->specular_color; std::sscanf( line.data(), "Ks %f %f %f", &Ks.r, &Ks.g, &Ks.b ) > 0 )
		{
		}
		else if( std::sscanf( line.data(), "d %f", &current_material->dissolved ) > 0 )
		{
		}
		else if( float Tr; std::sscanf( line.data(), "Tr %f", &Tr ) > 0 )
		{
			current_material->dissolved = 1.0f - Tr;
		}
		else if( std::sscanf( line.data(), "Ni %f", &current_material->optical_density ) > 0 )
		{
		}
		else if( int illum; std::sscanf( line.data(), "illum %d", &illum ) > 0 )
		{
			current_material->illumination_model = static_cast< IlluminationModel >( illum );
		}
	}

	if( current_material )
		materials_.push_back( *current_material );
}

ORB_NAMESPACE_END
