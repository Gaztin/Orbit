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
#include "Orbit/Graphics/Shader/Generator/Swizzle.h"

#include <map>
#include <sstream>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	IVariable::IVariable( const IVariable& other )
		: m_value( other.m_value )
		, m_type ( other.m_type )
	{
		other.SetUsed();
	}

	IVariable::IVariable( IVariable&& other )
		: m_value ( std::move( other.m_value ) )
		, m_type  ( other.m_type )
		, m_stored( other.m_stored )
		, m_used  ( other.m_used )
	{
		other.m_type   = VariableType::Unknown;
		other.m_stored = false;
		other.m_used   = false;
	}

	IVariable::IVariable( double f )
		: m_value ( std::to_string( f ) )
		, m_type  ( VariableType::Float )
		, m_stored( false )
		, m_used  ( false )
	{
	}

	IVariable::IVariable( std::string_view value, VariableType type )
		: m_value( value )
		, m_type ( type )
	{
	}

	void IVariable::StoreValue( void )
	{
		if( !m_stored )
		{
			static uint32_t unique_index = 0;

			std::ostringstream ss;
			ss << "local_" << ( unique_index++ );

			auto value = m_value;
			m_value = ss.str();

			const std::string typestring( VariableTypeToString( m_type ) );
			IGenerator::AppendSourceCode( "\t" + typestring + " " + m_value + " = " + value + ";\n" );

			m_stored = true;
		}
	}

	IVariable IVariable::operator*( const IVariable& rhs ) const
	{
		SetUsed();
		rhs.SetUsed();

		switch( IGenerator::GetLanguage() )
		{
			case ShaderLanguage::HLSL:
			{
				return IVariable( "mul( " + rhs.GetValue() + ", " + GetValue() + " )", GetType() );
			} break;

			default:
			{
				return IVariable( "( " + GetValue() + " * " + rhs.GetValue() + " )", GetType() );
			} break;
		}
	}

	IVariable IVariable::operator+( const IVariable& rhs ) const
	{
		SetUsed();
		rhs.SetUsed();

		return IVariable( "( " + GetValue() + " + " + rhs.GetValue() + " )", m_type );
	}

	IVariable IVariable::operator-( void ) const
	{
		SetUsed();

		return IVariable( "( -" + GetValue() + " )", m_type );
	}

	Swizzle* IVariable::operator->( void )
	{
		static Swizzle swizzle;

		SetUsed();

		Swizzle::latest_accessed_variable = this;
		return &swizzle;
	}

	void IVariable::operator=( const IVariable& rhs )
	{
		rhs.SetUsed();

		StoreValue();
		IGenerator::AppendSourceCode( "\t" + GetValue() + " = " + rhs.GetValue() + ";\n" );
	}

	void IVariable::operator+=( const IVariable& rhs )
	{
		rhs.SetUsed();

		StoreValue();
		IGenerator::AppendSourceCode( "\t" + GetValue() + " += " + rhs.GetValue() + ";\n" );
	}

	void IVariable::operator*=( const IVariable& rhs )
	{
		rhs.SetUsed();

		/* TODO: if m_type == VariableType::Mat4, do mul for HLSL */

		StoreValue();
		IGenerator::AppendSourceCode( "\t" + GetValue() + " *= " + rhs.GetValue() + ";\n" );
	}
}

ORB_NAMESPACE_END
