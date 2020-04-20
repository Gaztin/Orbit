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

#include "Uniform.h"

#include "Orbit/ShaderGen/Generator/IGenerator.h"

#include <cassert>
#include <sstream>

ORB_NAMESPACE_BEGIN

namespace ShaderGen { namespace Variables
{
	static std::string NewName( size_t unique_index )
	{
		std::ostringstream ss;
		ss << "uniform_" << unique_index;

		return ss.str();
	}

	UniformBase::UniformBase( DataType type )
		: IVariable( NewName( IGenerator::GetCurrentGenerator()->uniforms_.size() ), type )
	{
		stored_ = true;

		IGenerator::GetCurrentGenerator()->uniforms_.push_back( this );
	}

	UniformArrayBase::UniformArrayBase( DataType element_type )
		: UniformBase   ( DataType::Array )
		, element_type_( element_type )
	{
	}

	IVariable UniformArrayBase::operator[]( const IVariable& index ) const
	{
		assert( index.GetDataType() == DataType::Int );

		SetUsed();
		index.SetUsed();

		return IVariable( GetValue() + "[ " + index.GetValue() + " ]", element_type_ );
	}
} }

ORB_NAMESPACE_END
