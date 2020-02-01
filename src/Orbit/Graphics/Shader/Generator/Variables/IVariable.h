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

#pragma once
#include "Orbit/Graphics/Shader/Generator/VariableType.h"

#include <string_view>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	struct Swizzle;

	class ORB_API_GRAPHICS IVariable
	{
		friend class IGenerator;

	public:

		IVariable( void ) = default;
		IVariable( const IVariable& other );
		IVariable( IVariable&& other );
		IVariable( double f );
		IVariable( std::string_view value, VariableType type );
		virtual ~IVariable( void ) = default;

	public:

		/* Stores the value in a local variable. Useful when we want to manipulate proxies within a
		 * variable, since `Vec2(1.0, 0.5).g *= 2.0;` is ill-behaved. */
		void StoreValue( void );

	public:

		VariableType GetType  ( void ) const { return m_type; };
		void         SetUsed  ( void ) const { m_used = true; }
		void         SetStored( void )       { m_stored = true; }
		bool         IsStored ( void ) const { return m_stored; };

	public:

		virtual std::string GetValue( void ) const { return m_value; }

	public:

		IVariable operator*( const IVariable& ) const;
		IVariable operator+( const IVariable& ) const;
		IVariable operator-( void )             const;

		Swizzle* operator->( void );
		void     operator= ( const IVariable& );
		void     operator+=( const IVariable& );
		void     operator*=( const IVariable& );

	protected:

		std::string  m_value;
		VariableType m_type;
		bool         m_stored = false;
		mutable bool m_used   = false;

	};

}

ORB_NAMESPACE_END
