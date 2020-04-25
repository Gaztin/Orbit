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
#include "Orbit/ShaderGen/ShaderGen.h"
#include "Orbit/Core/Utility/Singleton.h"

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	class  IShader;
	struct MainFunction;

	class ORB_API_SHADERGEN ShaderManager : public Singleton< ShaderManager >
	{
	public:

		IShader*      GetCurrentShader      ( void ) const                  { return current_shader_; }
		MainFunction* GetCurrentMainFunction( void ) const                  { return current_main_function_; }
		void          SetCurrentShader      ( IShader* shader )             { current_shader_ = shader; }
		void          SetCurrentMainFunction( MainFunction* main_function ) { current_main_function_ = main_function; }

	private:

		IShader*      current_shader_;
		MainFunction* current_main_function_;

	};
}

ORB_NAMESPACE_END
