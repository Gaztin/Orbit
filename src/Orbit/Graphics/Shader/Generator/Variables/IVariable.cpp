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

#include "IVariable.h"

#include "Orbit/Graphics/Shader/Generator/Variables/Float.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Mat4.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Vec2.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Vec3.h"
#include "Orbit/Graphics/Shader/Generator/Variables/Vec4.h"
#include "Orbit/Graphics/Shader/Generator/IGenerator.h"
#include "Orbit/Graphics/Shader/Generator/MainFunction.h"
#include "Orbit/Graphics/Shader/Generator/Swizzle.h"

#include <cassert>
#include <map>
#include <sstream>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen { namespace Variables
{
	IVariable::IVariable( const IVariable& other )
		: m_value    ( other.m_value )
		, m_data_type( other.m_data_type )
	{
		other.SetUsed();
	}

	IVariable::IVariable( IVariable&& other )
		: m_value    ( std::move( other.m_value ) )
		, m_data_type( other.m_data_type )
		, m_stored   ( other.m_stored )
		, m_used     ( other.m_used )
	{
		other.m_data_type   = DataType::Unknown;
		other.m_stored = false;
		other.m_used   = false;
	}

	IVariable::IVariable( double f )
		: m_value    ( std::to_string( f ) )
		, m_data_type( DataType::Float )
		, m_stored   ( false )
		, m_used     ( false )
	{
	}

	IVariable::IVariable( int i )
		: m_value    ( std::to_string( i ) )
		, m_data_type( DataType::Int )
		, m_stored   ( false )
		, m_used     ( false )
	{
	}

	IVariable::IVariable( std::string_view value, DataType data_type )
		: m_value    ( value )
		, m_data_type( data_type )
	{
	}

	void IVariable::StoreValue( void )
	{
		if( !m_stored )
		{
			static uint32_t unique_index = 0;

			std::ostringstream ss;
			ss << "local_" << ( unique_index++ );

			auto old_value = m_value;
			m_value = ss.str();

			IGenerator::GetCurrentMainFunction()->code << "\t" << DataTypeToString( m_data_type ) << " " << m_value << " = " << old_value << ";\n";

			m_stored = true;
		}
	}

	IVariable IVariable::operator*( const IVariable& rhs ) const
	{
		assert( ( m_data_type == rhs.m_data_type ) ||
		        ( m_data_type == DataType::Mat4  && ( rhs.m_data_type == DataType::FVec4                                                                             ) ) ||
		        ( m_data_type == DataType::FVec4 && ( rhs.m_data_type == DataType::Mat4  || rhs.m_data_type == DataType::Float                                       ) ) ||
		        ( m_data_type == DataType::FVec3 && ( rhs.m_data_type == DataType::Float                                                                             ) ) ||
		        ( m_data_type == DataType::FVec2 && ( rhs.m_data_type == DataType::Float                                                                             ) ) ||
		        ( m_data_type == DataType::Float && ( rhs.m_data_type == DataType::FVec4 || rhs.m_data_type == DataType::FVec3 || rhs.m_data_type == DataType::FVec2 ) ) );

		SetUsed();
		rhs.SetUsed();

		DataType result_type = DataType::Unknown;
		switch( m_data_type )
		{
			case DataType::Float:
			case DataType::Mat4:
			{
				result_type = rhs.m_data_type;
			} break;

			case DataType::FVec2:
			case DataType::FVec3:
			case DataType::FVec4:
			{
				result_type = m_data_type;
			} break;
		}

		if( IGenerator::GetCurrentMainFunction()->shader_language == ShaderLanguage::HLSL &&
		    ( m_data_type == DataType::Mat4 || rhs.m_data_type == DataType::Mat4 ) )
		{
			return IVariable( "mul( " + rhs.GetValue() + ", " + GetValue() + " )", result_type );
		}

		return IVariable( "( " + GetValue() + " * " + rhs.GetValue() + " )", result_type );
	}

	IVariable IVariable::operator/( const IVariable& rhs ) const
	{
		assert( ( ( m_data_type == DataType::Float ) || ( m_data_type == DataType::FVec2 ) || ( m_data_type == DataType::FVec3 ) || ( m_data_type == DataType::FVec4 ) ) &&
		        ( rhs.m_data_type == DataType::Float ) );

		SetUsed();
		rhs.SetUsed();

		return IVariable( "( " + GetValue() + " / " + rhs.GetValue() + " )", m_data_type );
	}

	IVariable IVariable::operator+( const IVariable& rhs ) const
	{
		SetUsed();
		rhs.SetUsed();

		return IVariable( "( " + GetValue() + " + " + rhs.GetValue() + " )", m_data_type );
	}

	IVariable IVariable::operator-( const IVariable& rhs ) const
	{
		SetUsed();
		rhs.SetUsed();

		return IVariable( "( " + GetValue() + " - " + rhs.GetValue() + " )", m_data_type );
	}

	IVariable IVariable::operator-( void ) const
	{
		SetUsed();

		return IVariable( "( -" + GetValue() + " )", m_data_type );
	}

	IVariable IVariable::operator[]( size_t index ) const
	{
		return ( *this )[ IVariable( static_cast< int >( index ) ) ];
	}

	SwizzlePermutations* IVariable::operator->( void )
	{
		static SwizzlePermutations swizzle;

		SetUsed();

		variable_to_be_swizzled = this;
		return &swizzle;
	}

	void IVariable::operator=( const IVariable& rhs )
	{
		rhs.SetUsed();

		StoreValue();
		IGenerator::GetCurrentMainFunction()->code << "\t" << GetValue() << " = " << rhs.GetValue() << ";\n";
	}

	void IVariable::operator+=( const IVariable& rhs )
	{
		rhs.SetUsed();

		StoreValue();
		IGenerator::GetCurrentMainFunction()->code << "\t" << GetValue() << " += " << rhs.GetValue() << ";\n";
	}

	void IVariable::operator*=( const IVariable& rhs )
	{
		rhs.SetUsed();

		/* TODO: if m_type == DataType::Mat4, do mul for HLSL */

		StoreValue();
		IGenerator::GetCurrentMainFunction()->code << "\t" << GetValue() << " *= " << rhs.GetValue() << ";\n";
	}

	IVariable IVariable::operator[]( const IVariable& index ) const
	{
		DataType data_type = DataType::Unknown;
		switch( m_data_type )
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

		SetUsed();

		std::ostringstream ss;
		ss << GetValue() << "[ " << index.GetValue() << " ]";

		return IVariable( ss.str(), data_type );
	}
} }

ORB_NAMESPACE_END
