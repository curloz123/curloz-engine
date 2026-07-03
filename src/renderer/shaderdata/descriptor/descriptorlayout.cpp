#include "renderer/shaderdata/descriptor/descriptorlayout.hpp"
#include "core/logs.hpp"
#include "renderer/shaderdata/descriptor/descriptor.hpp"
#include "renderer/shaderdata/descriptor/texture.hpp"
#include "renderer/shaderdata/descriptor/uniformbuffers.hpp"
#include "renderer/vk_types.hpp"
#include <array>
#include <vulkan/vulkan.h>

namespace clz::renderer
{

	bool createDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		// Descriptor type - 1
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = r_TRANSFORM_DESCRIPTOR_BINDING;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings.push_back(uboLayoutBinding);

		// Descriptor type - 2
		VkDescriptorSetLayoutBinding textureLayoutBinding = {};
		textureLayoutBinding.binding = r_TEXTURE_DESCRIPTOR_BIND_POINT;
		textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureLayoutBinding.descriptorCount = r_MAX_TEXTURE_COUNT;
		textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings.push_back(textureLayoutBinding);

		constexpr std::array<VkDescriptorBindingFlags, 2> bindingFlags = {
		    0,											    // Binding 0 - no flags for transform UBO
		    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT // For textures
		};
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = bindingFlags.size();
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = &bindingFlagsInfo;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(clz::renderer::r_deviceContext.device, &layoutInfo, nullptr, &r_descriptorSetLayout) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not initialize descriptor set layout");
			return false;
		}
		clz::log::info("Initialized descriptor set layout");
		return true;
	}

	void destroyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(clz::renderer::r_deviceContext.device, r_descriptorSetLayout, nullptr);
	}
} // namespace clz::renderer
