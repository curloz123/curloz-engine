/**
 * @file device_properties.hpp
 * @brief Provides utility functions to query and cache Vulkan physical device properties.
 *
 * This file contains inline functions to retrieve hardware-specific limits, such as
 * the maximum supported sampler anisotropy and the maximum supported MSAA sample counts.
 */

#pragma once
#include <vulkan/vulkan.h>
#include "renderer/vk_types.hpp"
#include <optional>
#include <vulkan/vulkan_core.h>
#include <bit>
#include "core/logs.hpp"

namespace
{
        /// @brief Cached maximum sampler anisotropy supported by the physical device.
        inline std::optional<float> maxAnisotropy;

        /// @brief Cached maximum MSAA sample count supported by the physical device.
        inline std::optional<VkSampleCountFlagBits> maxMsaaSamples;
}

namespace clz::renderer
{
        /**
         * @brief Retrieves the maximum sampler anisotropy supported by the physical device.
         * 
         * Queries the physical device properties on the first call and caches the result
         * for subsequent calls to avoid redundant Vulkan API calls.
         * 
         * @return The maximum supported sampler anisotropy value.
         */
        inline float getMaxAnisotropy()
        {
                if (!maxAnisotropy.has_value())
                {
                        VkPhysicalDeviceProperties2 properties = {};
                        vkGetPhysicalDeviceProperties2(r_deviceContext.physicalDevice, &properties);
                        maxAnisotropy = properties.properties.limits.maxSamplerAnisotropy;
                }

                return maxAnisotropy.value();
        }

        /**
         * @brief Retrieves the maximum supported MSAA sample count for both color and depth framebuffers.
         * 
         * Queries the physical device properties on the first call, finds the highest common
         * sample count supported for both color and depth attachments, and caches the result.
         * 
         * @return The maximum supported VkSampleCountFlagBits.
         */
        inline VkSampleCountFlagBits getMaxMsaaSamples()
        {
                if (!maxMsaaSamples.has_value())
                {
                        VkPhysicalDeviceProperties physicalDeviceProperties;
                        vkGetPhysicalDeviceProperties(
                                        r_deviceContext.physicalDevice,
                                        &physicalDeviceProperties);

                        VkSampleCountFlags counts =
                                physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                physicalDeviceProperties.limits.framebufferDepthSampleCounts;

                        if (counts & VK_SAMPLE_COUNT_64_BIT)
                                maxMsaaSamples = VK_SAMPLE_COUNT_64_BIT;
                        else if (counts & VK_SAMPLE_COUNT_32_BIT)
                                maxMsaaSamples = VK_SAMPLE_COUNT_32_BIT;
                        else if (counts & VK_SAMPLE_COUNT_16_BIT)
                                maxMsaaSamples = VK_SAMPLE_COUNT_16_BIT;
                        else if (counts & VK_SAMPLE_COUNT_8_BIT)
                                maxMsaaSamples = VK_SAMPLE_COUNT_8_BIT;
                        else if (counts & VK_SAMPLE_COUNT_4_BIT)
                                maxMsaaSamples = VK_SAMPLE_COUNT_4_BIT;
                        else if (counts & VK_SAMPLE_COUNT_2_BIT)
                                maxMsaaSamples = VK_SAMPLE_COUNT_2_BIT;
                        else
                                maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
                }
                return maxMsaaSamples.value();
        }

        /**
         * @brief Converts an integer sample count to the corresponding VkSampleCountFlagBits.
         * 
         * Takes an integer representing the number of samples (e.g., 1, 2, 4, 8) and returns
         * the matching Vulkan enum flag. If the integer is not a valid power of two or exceeds
         * the maximum supported bits, it logs a warning and defaults to 1 sample.
         * 
         * @param flagInt The integer sample count (expected to be a power of 2).
         * @return The corresponding VkSampleCountFlagBits.
         */
        inline VkSampleCountFlagBits getMsaaFlagBitsFromInt(uint32_t flagInt)
        {
                flagInt = std::bit_floor(flagInt);
                switch(flagInt)
                {
                case(1):
                        return VK_SAMPLE_COUNT_1_BIT;
                case(2):
                        return VK_SAMPLE_COUNT_2_BIT;
                case(4):
                        return VK_SAMPLE_COUNT_4_BIT;
                case(8):
                        return VK_SAMPLE_COUNT_8_BIT;
                case(16):
                        return VK_SAMPLE_COUNT_16_BIT;
                case(32):
                        return VK_SAMPLE_COUNT_32_BIT;
                default:
                        clz::log::warn("Vulkan Msaa invalid flag bits enquired, returning 1 bit flag");
                }

                return VK_SAMPLE_COUNT_1_BIT;
        }
        /**
         * @brief Converts VkSampleCountFlagBits to integer sample count
         * 
         * @param Vulkan's msaa sample flag bits
         * @return The corresponding MSAA integer value
         */
        inline uint32_t getIntFromMsaaFlagBits(VkSampleCountFlagBits msaaFlagBits)
        {
                switch(msaaFlagBits)
                {
		case VK_SAMPLE_COUNT_1_BIT:
			return 1;
		case VK_SAMPLE_COUNT_2_BIT:
			return 2;
		case VK_SAMPLE_COUNT_4_BIT:
			return 4;
		case VK_SAMPLE_COUNT_8_BIT:
			return 8;
		case VK_SAMPLE_COUNT_16_BIT:
			return 16;
		case VK_SAMPLE_COUNT_32_BIT:
			return 32;
                default:
                        clz::log::warn("Vulkan Msaa invalid flag bits sent, returning 1 bit flag");
                }

                return 1;
        }

} // namespace clz::renderer
