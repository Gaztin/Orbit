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

#include "Variable.h"

#include "Orbit/ShaderGen/Generator/IShader.h"
#include "Orbit/ShaderGen/Generator/MainFunction.h"
#include "Orbit/ShaderGen/Generator/ShaderManager.h"
#include "Orbit/ShaderGen/Variables/Float.h"
#include "Orbit/ShaderGen/Variables/Mat4.h"
#include "Orbit/ShaderGen/Variables/Swizzle.h"
#include "Orbit/ShaderGen/Variables/Vec2.h"
#include "Orbit/ShaderGen/Variables/Vec3.h"
#include "Orbit/ShaderGen/Variables/Vec4.h"

#include <cassert>
#include <map>
#include <sstream>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	Variable::Variable( const Variable& other )
		: value_    ( other.value_ )
		, data_type_( other.data_type_ )
	{
	}

	Variable::Variable( Variable&& other )
		: value_    ( std::move( other.value_ ) )
		, data_type_( other.data_type_ )
		, stored_   ( other.stored_ )
		, used_     ( other.used_ )
	{
		other.data_type_   = DataType::Unknown;
		other.stored_ = false;
		other.used_   = false;
	}

	Variable::Variable( double f )
		: value_    ( std::to_string( f ) )
		, data_type_( DataType::Float )
		, stored_   ( false )
		, used_     ( false )
	{
	}

	Variable::Variable( int i )
		: value_    ( std::to_string( i ) )
		, data_type_( DataType::Int )
		, stored_   ( false )
		, used_     ( false )
	{
	}

	Variable::Variable( std::string_view value, DataType data_type )
		: value_    ( value )
		, data_type_( data_type )
	{
	}

	void Variable::StoreValue( void )
	{
		if( !stored_ )
		{
			value_  = ShaderManager::GetInstance().NewLocal( data_type_, value_ );
			stored_ = true;
		}
	}

	Variable Variable::operator*( const Variable& rhs ) const
	{
		assert( ( data_type_ == rhs.data_type_ ) ||
		        ( data_type_ == DataType::Mat4  && ( rhs.data_type_ == DataType::FVec4                                                                             ) ) ||
		        ( data_type_ == DataType::FVec4 && ( rhs.data_type_ == DataType::Mat4  || rhs.data_type_ == DataType::Float                                       ) ) ||
		        ( data_type_ == DataType::FVec3 && ( rhs.data_type_ == DataType::Float                                                                             ) ) ||
		        ( data_type_ == DataType::FVec2 && ( rhs.data_type_ == DataType::Float                                                                             ) ) ||
		        ( data_type_ == DataType::Float && ( rhs.data_type_ == DataType::FVec4 || rhs.data_type_ == DataType::FVec3 || rhs.data_type_ == DataType::FVec2 ) ) );

		DataType result_type = DataType::Unknown;
		switch( data_type_ )
		{
			case DataType::Float:
			case DataType::Mat4:
			{
				result_type = rhs.data_type_;
			} break;

			case DataType::FVec2:
			case DataType::FVec3:
			case DataType::FVec4:
			{
				result_type = data_type_;
			} break;

			default: break;
		}

		if( ShaderManager::GetInstance().GetLanguage() == ShaderLanguage::HLSL &&
		    ( data_type_ == DataType::Mat4 || rhs.data_type_ == DataType::Mat4 ) )
		{
			return Variable( "mul( " + rhs.GetValue() + ", " + GetValue() + " )", result_type );
		}

		return Variable( "( " + GetValue() + " * " + rhs.GetValue() + " )", result_type );
	}

	Variable Variable::operator/( const Variable& rhs ) const
	{
		assert( ( ( data_type_ == DataType::Float ) || ( data_type_ == DataType::FVec2 ) || ( data_type_ == DataType::FVec3 ) || ( data_type_ == DataType::FVec4 ) ) &&
		        ( rhs.data_type_ == DataType::Float ) );

		return Variable( "( " + GetValue() + " / " + rhs.GetValue() + " )", data_type_ );
	}

	Variable Variable::operator+( const Variable& rhs ) const
	{
		return Variable( "( " + GetValue() + " + " + rhs.GetValue() + " )", data_type_ );
	}

	Variable Variable::operator-( const Variable& rhs ) const
	{
		return Variable( "( " + GetValue() + " - " + rhs.GetValue() + " )", data_type_ );
	}

	Variable Variable::operator-( void ) const
	{
		return Variable( "( -" + GetValue() + " )", data_type_ );
	}

	Variable Variable::operator[]( size_t index ) const
	{
		return ( *this )[ Variable( static_cast< int >( index ) ) ];
	}

	SwizzlePermutations* Variable::operator->( void ) const
	{
		static SwizzlePermutations swizzle;

		used_                   = true;
		variable_to_be_swizzled = const_cast< Variable* >( this );

		return &swizzle;
	}

	void Variable::operator=( const Variable& rhs )
	{
		assert( data_type_ == rhs.data_type_ );

		StoreValue();
		ShaderManager::GetInstance().Append() << "\t" << GetValue() << " = " << rhs.GetValue() << ";\n";
	}

	void Variable::operator+=( const Variable& rhs )
	{
		StoreValue();
		ShaderManager::GetInstance().Append() << "\t" << GetValue() << " += " << rhs.GetValue() << ";\n";
	}

	void Variable::operator*=( const Variable& rhs )
	{
		/* TODO: if m_type == DataType::Mat4, do mul for HLSL */

		StoreValue();
		ShaderManager::GetInstance().Append() << "\t" << GetValue() << " *= " << rhs.GetValue() << ";\n";
	}

	Variable Variable::operator[]( const Variable& index ) const
	{
		DataType data_type = DataType::Unknown;
		switch( data_type_ )
		{
			default: { assert( false ); } break;

			case DataType::FVec2:
			case DataType::FVec3:
			case DataType::FVec4:
			{
				data_type = DataType::Float;
			} break;

			case DataType::IVec2:
			case DataType::IVec3:
			case DataType::IVec4:
			{
				data_type = DataType::Int;
			} break;

			case DataType::Mat4:
			{
				data_type = DataType::FVec4;
			} break;
		}

		std::ostringstream ss;
		ss << GetValue() << "[ " << index.GetValue() << " ]";

		return Variable( ss.str(), data_type );
	}
}

ORB_NAMESPACE_END
