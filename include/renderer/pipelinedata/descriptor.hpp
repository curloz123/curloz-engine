/**
 * @file descriptor.hpp
 * @author curl0z
 * @brief Contains the descriptor pool and per-UBO / per-sampler descriptor
 * sets used by the main render pipeline.
 *
 * @note Each UBO (see ubo.hpp) and the combined image sampler (see
 * sampler.hpp) gets its OWN VkDescriptorSetLayout and its own set of
 * per-frame-in-flight VkDescriptorSets. This keeps them independently
 * bindable (and independently reusable across other pipelines that share
 * a layout-compatible binding, e.g. the editor's offscreen pipeline) rather
 * than being bundled into one monolithic descriptor set.
 */

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
/// @brief Single pool backing every descriptor set allocated below.
inline VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

/// @brief One descriptor set per frame-in-flight, bound to cameraUBOLayout.
inline std::vector<VkDescriptorSet> cameraDescriptorSets = {};

/// @brief One descriptor set per frame-in-flight, bound to combinedSamplerLayout.
inline std::vector<VkDescriptorSet> samplerDescriptorSets = {};

/// --- Seperate descriptor
} // namespace clz::renderer

namespace clz::renderer
{
/// @brief Creates the descriptor pool, allocates cameraDescriptorSets and
/// samplerDescriptorSets from their respective layouts, and writes the
/// camera UBO's buffer bindings. Texture/sampler bindings are written
/// separately via updateSamplersDataForDescriptorSets, once scene
/// textures are loaded.
/// @note Requires createUniformBuffers() and createCombinedSamplersLayout()
/// to have already run, since their layouts are consumed here.
bool createDescriptors();

/// @brief Destroys the descriptor pool (which implicitly frees every
/// descriptor set allocated from it).
void destroyDescriptors();
} // namespace clz::renderer