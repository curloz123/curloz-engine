#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace clz::editor::backend
{
inline VkDescriptorPool descriptorPool;
inline std::vector<VkDescriptorSet> cameraDescriptorSets;
} // namespace clz::editor::backend

namespace clz::editor::backend
{
bool createEditorDescriptors();
void destroyEditorDescriptors();
} // namespace clz::editor::backend