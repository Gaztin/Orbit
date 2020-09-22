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
#include "Orbit/ShaderGen/Variables/DataType.h"

#include <string_view>
#include <string>

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	struct SwizzlePermutations;

	class ORB_API_SHADERGEN Variable
	{
	public:

		Variable( void ) = default;
		Variable( const Variable& other );
		Variable( Variable&& other );
		Variable( double f );
		Variable( int i );
		Variable( std::string_view value, DataType data_type );
		virtual ~Variable( void ) = default;

	public:

		/* Stores the value in a local variable. Useful when we want to manipulate swizzles within
		 * a variable, since `Vec2(1.0, 0.5).g *= 2.0;` is ill-behaved. */
		void StoreValue( void );

		/** Fetch the value of this variable. Marks this variable as used. */
		std::string GetValue( void ) const;

	public:

		DataType GetDataType( void )        const { return data_type_; };
		bool     IsUsed     ( void )        const { return used_; }
		void     SetUsed    ( bool used )   const { used_ = used; }
		bool     IsStored   ( void )        const { return stored_; };
		void     SetStored  ( bool stored )       { stored_ = stored; }

	public:

		Variable operator* ( const Variable& rhs )   const;
		Variable operator/ ( const Variable& rhs )   const;
		Variable operator+ ( const Variable& rhs )   const;
		Variable operator- ( const Variable& rhs )   const;
		Variable operator- ( void )                   const;
		Variable operator[]( size_t index )           const;

		SwizzlePermutations* operator->( void ) const;

		void     operator= ( const Variable& rhs );
		void     operator+=( const Variable& rhs );
		void     operator*=( const Variable& rhs );

	public:

		virtual Variable operator[]( const Variable& index ) const;

	protected:

		virtual std::string GetValueDerived( void ) const { return value_; }

	protected:

		std::string  value_;
		DataType     data_type_;
		bool         stored_ = false;
		mutable bool used_   = false;

	};

	inline Variable operator+( double lhs, const Variable& rhs ) { return ( Variable( lhs ) + rhs ); }
	inline Variable operator-( double lhs, const Variable& rhs ) { return ( Variable( lhs ) - rhs ); }
	inline Variable operator*( double lhs, const Variable& rhs ) { return ( Variable( lhs ) * rhs ); }
	inline Variable operator/( double lhs, const Variable& rhs ) { return ( Variable( lhs ) / rhs ); }

}

ORB_NAMESPACE_END
