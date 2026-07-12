#pragma once

#include "renderer/entitydata/texture.hpp"
#include "math/mat4x4.hpp"

namespace clz::renderer
{
	struct ModelDataPC
	{
		math::mat4 modelMatrix;
		TextureID textureID;
	};

	constexpr uint32_t TEXTURE_DESCRIPTOR_BIND_POINT = 0;

}