#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer
{
void setHandleName(uint64_t handle, VkObjectType objectType, const char* name);
}