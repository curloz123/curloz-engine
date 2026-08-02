#pragma once

#include "math/mat4x4.hpp"
#include "renderer/entitydata/texture.hpp"

namespace clz::renderer
{

struct ModelDataPC
{
	math::mat4 modelMatrix;
	TextureID textureID;
};
} // namespace clz::renderer