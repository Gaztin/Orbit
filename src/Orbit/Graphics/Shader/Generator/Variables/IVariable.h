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
#include "Orbit/Graphics/Shader/Generator/DataType.h"

#include <string_view>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen { class IGenerator; struct SwizzlePermutations; namespace Variables
{
	class ORB_API_GRAPHICS IVariable
	{
		friend class ShaderGen::IGenerator;

	public:

		IVariable( void ) = default;
		IVariable( const IVariable& other );
		IVariable( IVariable&& other );
		IVariable( double f );
		IVariable( int i );
		IVariable( std::string_view value, DataType data_type );
		virtual ~IVariable( void ) = default;

	public:

		/* Stores the value in a local variable. Useful when we want to manipulate swizzles within
		 * a variable, since `Vec2(1.0, 0.5).g *= 2.0;` is ill-behaved. */
		void StoreValue( void );

	public:

		DataType GetDataType( void ) const { return data_type_; };
		void     SetUsed    ( void ) const { used_ = true; }
		void     SetStored  ( void )       { stored_ = true; }
		bool     IsStored   ( void ) const { return stored_; };

	public:

		virtual std::string GetValue( void ) const { return value_; }

	public:

		IVariable operator* ( const IVariable& rhs )   const;
		IVariable operator/ ( const IVariable& rhs )   const;
		IVariable operator+ ( const IVariable& rhs )   const;
		IVariable operator- ( const IVariable& rhs )   const;
		IVariable operator- ( void )                   const;
		IVariable operator[]( size_t index )           const;

		SwizzlePermutations* operator->( void );
		void     operator= ( const IVariable& rhs );
		void     operator+=( const IVariable& rhs );
		void     operator*=( const IVariable& rhs );

	public:

		virtual IVariable operator[]( const IVariable& index ) const;

	protected:

		std::string  value_;
		DataType     data_type_;
		bool         stored_ = false;
		mutable bool used_   = false;

	};

	inline IVariable operator+( double lhs, const IVariable& rhs ) { return ( IVariable( lhs ) + rhs ); }
	inline IVariable operator-( double lhs, const IVariable& rhs ) { return ( IVariable( lhs ) - rhs ); }
	inline IVariable operator*( double lhs, const IVariable& rhs ) { return ( IVariable( lhs ) * rhs ); }
	inline IVariable operator/( double lhs, const IVariable& rhs ) { return ( IVariable( lhs ) / rhs ); }

} }

ORB_NAMESPACE_END
