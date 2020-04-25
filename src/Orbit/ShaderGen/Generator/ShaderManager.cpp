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

#include "ShaderManager.h"

#include "Orbit/ShaderGen/Generator/IShader.h"
#include "Orbit/ShaderGen/Generator/MainFunction.h"

ORB_NAMESPACE_BEGIN

namespace ShaderGen
{
	std::ostringstream& ShaderManager::Append( void ) const
	{
		return current_main_function_->code;
	}

	std::string ShaderManager::NewLocal( DataType type, std::string_view code ) const
	{
		std::string name = "local_" + current_main_function_->locals_count;

		++current_main_function_->locals_count;
		current_main_function_->code << "\t" << DataTypeToString( type ) << " " << name << " = " << code << ";\n";

		return name;
	}

	std::string ShaderManager::NewAttribute( VertexComponent component ) const
	{
		const size_t attribute_index = current_shader_->attribute_layout_.GetCount();

		current_shader_->attribute_layout_.Add( component );

		return "attribute_" + std::to_string( attribute_index );
	}

	std::string ShaderManager::NewVarying( VertexComponent component ) const
	{
		const size_t varying_index = current_shader_->varying_layout_.GetCount();

		current_shader_->varying_layout_.Add( component );

		return "varying_" + std::to_string( varying_index );
	}

	std::string ShaderManager::NewSampler( void ) const
	{
		const size_t sampler_index = current_shader_->sampler_count_;

		++current_shader_->sampler_count_;

		return "sampler_" + std::to_string( sampler_index );
	}

	std::string ShaderManager::NewUniform( UniformBase* uniform ) const
	{
		const size_t uniform_index = current_shader_->uniforms_.size();

		current_shader_->uniforms_.push_back( uniform );

		return "uniform_" + std::to_string( uniform_index );
	}

	ShaderLanguage ShaderManager::GetLanguage( void ) const
	{
		return current_main_function_->shader_language;
	}

	ShaderType ShaderManager::GetType( void ) const
	{
		return current_main_function_->shader_type;
	}
}

ORB_NAMESPACE_END
