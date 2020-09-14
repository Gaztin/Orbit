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
#include "Orbit/Core/IO/File/Markup/XML/XMLFile.h"
#include "Orbit/Graphics/Animation/Joint.h"
#include "Orbit/Graphics/Geometry/Geometry.h"

#include <map>
#include <memory>
#include <vector>

ORB_NAMESPACE_BEGIN

class  VertexLayout;
struct KeyFrame;

class ORB_API_GRAPHICS COLLADAFile : public XMLFile
{
public:

	struct ModelData
	{
		std::vector< std::shared_ptr< Mesh > > meshes;

		Joint root_joint;
	};

	struct AnimationData
	{
		std::map< std::string, std::vector< KeyFrame > > keyframes;
	};

public:

	explicit COLLADAFile( ByteSpan data, const VertexLayout& vertex_layout );

public:

	ModelData     GetModelData     ( void ) const { return model_data_; }
	AnimationData GetAnimationData ( void ) const { return animation_data_; }

private:

	static Joint ColladaParseNodeRecursive( const XMLElement& node, const Matrix4& parent_inverse_bind_transform, const std::vector< std::string >& all_joint_names, const std::vector< Matrix4 >& all_joint_transforms );

private:

	ModelData     model_data_;
	AnimationData animation_data_;

};

ORB_NAMESPACE_END
