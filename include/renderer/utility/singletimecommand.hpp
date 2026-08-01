#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer
{
VkCommandBuffer startSingleTimeCommand();
void submitSingleTimeCommand(VkCommandBuffer commandBuffer);
} // namespace clz::renderer
