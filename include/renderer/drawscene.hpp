/**
 * @file drawscene.hpp
 * @author curl0z
 * @brief Main scene drawing file
 */
#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer
{
/// @brief draws the main scene
/// In editor mode, the image will be drawn as an offsreen image for editor window
/// In game mode, the scene is drawn as pure swapchain image
/// @param commandBuffer Active command buffer
void drawScene(VkCommandBuffer commandBuffer);
} // namespace clz::renderer