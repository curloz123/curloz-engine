/**
 * @file ubo.hpp
 * @author curl0z
 *
 * @brief pipeline's uniform buffers implementation is defined here.
 * File is divided into 3 parts
 * 1. Data Structure definition, General ubo struct and its memory
 * 2. Actual uniform buffers and layouts
 * 3. Function related to UBO's like creation, updating, deletion etc.
 *
 * @note The data structures, and layouts are also used by editor.
 * So think twice before changing anything
 */
#pragma once

#include "math/mat4x4.hpp"
#include "renderer/utility/memory.hpp"
#include <vector>

/// @brief --- 1. Uniform buffer Data Structure ---
namespace clz::renderer
{
	struct UBOMemory
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		void* mappedMemory;
		VkDeviceSize memorySize;
	};

	struct UBO
	{
		VkDeviceSize uboSize;
		uint32_t uboBindingPoint;
		std::vector<VkDeviceSize> offsets;
		std::vector<void*> mapped;

		explicit UBO(const VkDeviceSize uboSize, const uint32_t uboBindingPoint)
		    : uboSize(uboSize), uboBindingPoint(uboBindingPoint), offsets({}), mapped({})
		{
		}
	};

} // namespace clz::renderer

/// --- @brief 2. Actual UBO's and layouts ---
namespace clz::renderer
{
	/// @brief Main Uniform buffer memory
	/// All ubo's share this memory
	inline UBOMemory uboMemory;

	/// @brief Uniform buffers below are defined in this order -
	/// 0. This is just defined once, Specify total numbers of uniform buffers
	/// 1. Their bind point in shader, make sure to make them constexpr and name them as ${NAME_OF_UBO}_UBO_BINDPOINT.
	/// 2. Their descriptor set layout, named as ${NAME_OF_UBO}UBOLayout.
	/// 3. The actual shader struct, make sure to name them as ${NAME_OF_UBO}ShaderUBO.
	/// 4. Their respective CPU side uniform data structure ${NAME_OF_UBO}UBO.
	///
	/// All the "pipeline-specific" uniform buffers, such as shape pipeline's seperate camera ubo etc.
	/// go at the bottom, also their update function should be managed seperately.

	/// --- 0. Number of UBO's ---
	inline constexpr uint8_t NUM_UNIFORM_BUFFERS = 1;

	/// --- 1. Camera UBO ---
	inline constexpr uint8_t CAMERA_UBO_BINDPOINT = 0;
	inline VkDescriptorSetLayout cameraUBOLayout = VK_NULL_HANDLE;
	struct CameraShaderUBO
	{
		math::mat4 projection;
		math::mat4 view;
	};
	inline UBO cameraUBO(sizeof(CameraShaderUBO), CAMERA_UBO_BINDPOINT);

} // namespace clz::renderer

/// --- 3. UBO funcions ---
namespace clz::renderer
{
	/// @brief Creates all uniform buffers
	/// And lastly create all their respective descriptor layouts
	/// Currently creates -
	/// 1. Camera UBO
	/// @note Must be called only before creating any pipeline
	bool createUniformBuffers();

	/// @brief Updates all uniform buffers
	/// Currently updates -
	/// 1. Camera UBO
	/// @note Must be called only once per frame before performing any vkCmdDraw
	void updateUniformBuffers(const CameraShaderUBO& shaderCameraUBO);

	/// @brief Destroys all uniform buffers (just deletes the main memory)
	void destroyUniformBuffers();
} // namespace clz::renderer