/**
 * @file devicecontext.cpp
 * @author curl0z
 * @brief Implementation of the initialization
 * and cleanup of device context
 */

#include "renderer/context/devicecontext.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include <algorithm>
#include <array>
#include <config/config.hpp>
#include <cstring>
#include <vector>
#include <vulkan/vulkan.h>
#include <window/vulkanhelper.hpp>

namespace clz::renderer
{
#ifdef CLZ_DEBUG
	constexpr bool r_enableValidationLayers = true;
#else
	constexpr bool r_enableValidationLayers = false;
#endif

	constexpr auto r_debugExtensionName = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	constexpr auto r_validationLayers   = "VK_LAYER_KHRONOS_validation";
	constexpr std::array<const char*, 1> r_requiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

} // namespace clz::renderer

namespace clz::renderer
{
	bool initDeviceContext()
	{
		if (!createInstance())
		{
			clz::log::error("Could not create instance");
			clz::log::error("Could not create device context");
			return false;
		}

		if (!createDebugMessenger())
		{
			clz::log::error("Could not create debug messenger");
			clz::log::error("Could not create device context");
			return false;
		}

		if (!createSurface())
		{
			clz::log::error("Could not create surface");
			clz::log::error("Could not create device context");
			return false;
		}

		if (!selectPhysicalDevice())
		{
			clz::log::error("Could not select any physical device");
			clz::log::error("Could not create device context");
			return false;
		}

		if (!createLogicalDevice())
		{
			clz::log::error("Could not create logical device");
			clz::log::error("Could not create device context");
			return false;
		}

		clz::log::info("renderer: initialized device context successfully");

		return true;
	}

	// Instance Creation Part

	/**
	 * @brief Retrieves all required instance extensions
	 * @param rRequiredExtensions
	 * @return true if retrieved all extensions succesfully, else prints clz::error and
	 * returns false
	 */
	bool getRequiredInstanceExtensions(std::vector<const char*>& rRequiredExtensions)
	{
		if (!clz::window::getRequiredVulkanExtensions(rRequiredExtensions))
		{
			clz::log::error("Unable to retrieve instance extensions from window");
			return false;
		}

		if (r_enableValidationLayers)
		{
			rRequiredExtensions.push_back(r_debugExtensionName);
		}

		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());

		for (const auto& requiredExtension : rRequiredExtensions)
		{
			const bool found = std::ranges::any_of(
				availableExtensions.begin(),
				availableExtensions.end(),
				[requiredExtension](const auto& availableExtension) 
				{
					return std::strcmp(
							availableExtension.extensionName,
						      	requiredExtension) == 0;
				}
			);
			if (!found)
			{
				clz::log::error(
					"Extension: " + std::string(requiredExtension) +
					" is not available"
				);
				return false;
			}
		}
		clz::log::debug("All required instance extensions present");
		return true;
	}

	bool getValidationLayers(std::vector<const char*>& rValidationLayers)
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (auto layer : rValidationLayers)
		{
			const bool found = std::ranges::any_of(
				availableLayers.begin(),
				availableLayers.end(),
				[&](auto availableLayer) {
					return std::strcmp(layer, availableLayer.layerName) == 0;
				}
			);
			if (!found)
			{
				clz::log::error("Layer: " + std::string(layer) + " not available");
				return false;
			}
		}

		clz::log::debug("All requested layers present");
		return true;
	}

	bool createInstance()
	{
		// Application Info
		VkApplicationInfo appInfo = {};

		// Define which type of object is this
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		// Name of our application
		const std::string appName = clz::config::getValue<std::string>("engine", "name", "Curloz Engine");
		appInfo.pApplicationName = appName.c_str();
		// Our application's version
		appInfo.applicationVersion = VK_MAKE_VERSION(
			clz::config::getValue<int>("engine", "version_major", 0),
			clz::config::getValue<int>("engine", "version_minor", 0),
			clz::config::getValue<int>("engine", "version_patch", 0)
		);

		// Our application's name
		appInfo.pEngineName = "Curloz Engine";
		// Which API version we want to use
		appInfo.apiVersion = VK_API_VERSION_1_3;

		// Instance info
		VkInstanceCreateInfo instanceInfo = {};

		// Struct type
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		// Our application's info
		instanceInfo.pApplicationInfo = &appInfo;

		// Get Extensions
		std::vector<const char*> requiredExtensions;
		if (!getRequiredInstanceExtensions(requiredExtensions))
		{
			clz::log::error("All required device extensions not present");
			return false;
		}

		// Send info about extensions
		instanceInfo.enabledExtensionCount = requiredExtensions.size();
		instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

		// Enable layers
		std::vector<const char*> layers;
		if (r_enableValidationLayers)
		{
			if (!getValidationLayers(layers))
			{
				clz::log::error("Validation layers not present");
				return false;
			}
			instanceInfo.enabledLayerCount = layers.size();
			instanceInfo.ppEnabledLayerNames = layers.data();
		}
		else
		{
			instanceInfo.enabledLayerCount = 0;
			instanceInfo.ppEnabledLayerNames = nullptr;
		}

		if (vkCreateInstance(&instanceInfo, nullptr, &r_deviceContext.instance) !=
		    VK_SUCCESS)
		{
			clz::log::error("Vulkan could not create instance");
			return false;
		}

		clz::log::info("renderer: created instance");

		return true;
	}

	// Debug messenger and validation layers
	VKAPI_ATTR VkBool32 VKAPI_CALL printMessage(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pMessageData,
		void* pUserData
	)
	{
		switch (messageSeverity)
		{
		case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT):
			clz::log::info(
				"VULKAN VALIDATION LAYERS: " + std::string(pMessageData->pMessage)
			);
			break;
		case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT):
			clz::log::warn(
				"VULKAN VALIDATION LAYERS: " + std::string(pMessageData->pMessage)
			);
			break;
		case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT):
			clz::log::error(
				"VULKAN VALIDATION LAYERS: " + std::string(pMessageData->pMessage)
			);
			// clz::CLZ_ASSERT(false, "fuck off");
			break;
		default:
			break;
		}
		return VK_FALSE;
	}

	bool createDebugMessenger()
	{
		if constexpr (!r_enableValidationLayers)
			return true;

		VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
		messengerInfo.sType 		= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messengerInfo.messageSeverity 	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
							VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
							VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		messengerInfo.messageType 	= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
					    		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					    		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		messengerInfo.pfnUserCallback = printMessage;
		messengerInfo.pUserData = nullptr; // Optional

		const auto createMessenger =
			reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
				r_deviceContext.instance,
				"vkCreateDebugUtilsMessengerEXT"
			));
		if (!createMessenger)
		{
			clz::log::error(
				"renderer: Could not find the debug messenger creator "
				"function"
			);
			return false;
		}

		if (createMessenger(
			    r_deviceContext.instance,
			    &messengerInfo,
			    nullptr,
			    &r_deviceContext.debugMessenger
		    ) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not create debug messenger");
			return false;
		}
		clz::log::info("renderer: Created Debug Messenger");

		return true;
	}

	bool createSurface()
	{
		if (auto surfaceResult = clz::window::createVulkanSurface(
			    r_deviceContext.instance,
			    r_deviceContext.surface
		    );
		    !surfaceResult)
		{
			clz::log::error("Vulkan could not create surface");
			return false;
		}

		clz::log::info("Created window surface");
		return true;
	}

	bool selectPhysicalDevice()
	{
		uint32_t numPhysicalDevices = 0;
		vkEnumeratePhysicalDevices(r_deviceContext.instance, &numPhysicalDevices, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);
		vkEnumeratePhysicalDevices(
			r_deviceContext.instance,
			&numPhysicalDevices,
			physicalDevices.data()
		);

		uint32_t maxScore = 0;
		for (const auto physicalDevice : physicalDevices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
			if (deviceProperties.apiVersion < VK_API_VERSION_1_3)
			{
				clz::log::warn(
					"Physical Device: " +
					std::string(deviceProperties.deviceName) +
					", Does not support vulkan 1.3"
				);
				continue;
			}

			// check graphics support
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(
				physicalDevice,
				&queueFamilyCount,
				nullptr
			);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(
				physicalDevice,
				&queueFamilyCount,
				queueFamilies.data()
			);

			bool graphicsSupport = false;
			bool presentSupport = false;
			for (size_t j = 0; j < queueFamilies.size(); ++j)
			{
				// Graphics Support
				if (!graphicsSupport &&
				    queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					r_deviceContext.graphicsFamily = j;
					graphicsSupport = true;
				}

				// Present Support
				if (!presentSupport)
				{
					VkBool32 presentFamilyFound = VK_FALSE;
					vkGetPhysicalDeviceSurfaceSupportKHR(
						physicalDevice,
						j,
						r_deviceContext.surface,
						&presentFamilyFound
					);
					if (presentFamilyFound == VK_TRUE)
					{
						r_deviceContext.presentFamily = j;
						presentSupport = true;
					}
				}
				if (graphicsSupport && presentSupport)
					break;
			}

			if (!graphicsSupport || !presentSupport)
			{
				clz::log::warn(
					"GPU: " + std::string(deviceProperties.deviceName) +
					"is not suitable"
				);
				continue;
			}

			// this device's local score
			uint32_t deviceScore = 0;

			// First parameter - Checking for maximum image size
			deviceScore += deviceProperties.limits.maxImageDimension2D;

			// Checking if the device is Discrete or not
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				deviceScore += 10000;

			// Final comparison with previous devices to see if current device
			// has a better score
			if (deviceScore > maxScore)
			{
				maxScore = deviceScore;
				r_deviceContext.physicalDevice = physicalDevice;
			}
		}

		if (r_deviceContext.physicalDevice == VK_NULL_HANDLE)
		{
			clz::log::error("No Suitable GPU found");
			return false;
		}
		VkPhysicalDeviceProperties selectedDeviceProperties;
		vkGetPhysicalDeviceProperties(
			r_deviceContext.physicalDevice,
			&selectedDeviceProperties
		);

		clz::log::info("Using GPU: " + std::string(selectedDeviceProperties.deviceName));

		return true;
	}

	bool checkDeviceExtensions()
	{
		uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(
			r_deviceContext.physicalDevice,
			nullptr,
			&deviceExtensionCount,
			nullptr
		);
		std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(
			r_deviceContext.physicalDevice,
			nullptr,
			&deviceExtensionCount,
			availableDeviceExtensions.data()
		);

		for (auto requiredDeviceExtension : r_requiredDeviceExtensions)
		{
			const bool found = std::ranges::any_of(
				availableDeviceExtensions,
				[&](auto availableExtension) {
					return std::strcmp(
						       requiredDeviceExtension,
						       availableExtension.extensionName
					       ) == 0;
				}
			);

			if (!found)
			{
				clz::log::error(
					"required vulkan device extension: " +
					std::string(requiredDeviceExtension) +
					" is not supported by the selected GPU"
				);
				return false;
			}
		}

		clz::log::info("All device extensions present");
		return true;
	}

	bool createLogicalDevice()
	{
		// Checking if all required device extensions are present??
		if (!checkDeviceExtensions())
		{
			clz::log::error(
				"Could not create logical device, as extensions are not "
				"present"
			);
			return false;
		}

		// Checking if queue indexes are same or different
		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		constexpr float queuePriority = 1.0f;
		if (r_deviceContext.graphicsFamily == r_deviceContext.presentFamily)
		{
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = r_deviceContext.graphicsFamily.value();
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;
			queueInfos.push_back(queueInfo);
		}
		else
		{
			VkDeviceQueueCreateInfo graphicsQueueInfo = {};
			graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			graphicsQueueInfo.queueFamilyIndex = r_deviceContext.graphicsFamily.value();
			graphicsQueueInfo.queueCount = 1;
			graphicsQueueInfo.pQueuePriorities = &queuePriority;

			VkDeviceQueueCreateInfo presentQueueInfo = {};
			presentQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			presentQueueInfo.queueFamilyIndex = r_deviceContext.presentFamily.value();
			presentQueueInfo.queueCount = 1;
			presentQueueInfo.pQueuePriorities = &queuePriority;

			queueInfos.push_back(graphicsQueueInfo);
			queueInfos.push_back(presentQueueInfo);
		}

		// Descriptor indexing - for textures
		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
		descriptorIndexingFeatures.sType =
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		descriptorIndexingFeatures.pNext = nullptr;
		descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

		// Vulkan 1.3 device Features — enable dynamic rendering
		VkPhysicalDeviceVulkan13Features features13 = {};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.pNext = &descriptorIndexingFeatures;
		features13.dynamicRendering = VK_TRUE;
		features13.synchronization2 = VK_TRUE;

		// Normal device features
		VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &features13; // chain features 13
		deviceFeatures2.features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = &deviceFeatures2; // chain all required features
		deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.enabledExtensionCount =
			static_cast<uint32_t>(r_requiredDeviceExtensions.size());
		deviceInfo.ppEnabledExtensionNames = r_requiredDeviceExtensions.data();

		if (vkCreateDevice(
			    r_deviceContext.physicalDevice,
			    &deviceInfo,
			    nullptr,
			    &r_deviceContext.device
		    ) != VK_SUCCESS)
		{
			clz::log::error("Failed to create renderer's logical device");
			return false;
		}
		clz::log::info("created vulkan logical device");

		vkGetDeviceQueue(
			r_deviceContext.device,
			r_deviceContext.graphicsFamily.value(),
			0,
			&r_deviceContext.graphicsQueue
		);
		vkGetDeviceQueue(
			r_deviceContext.device,
			r_deviceContext.presentFamily.value(),
			0,
			&r_deviceContext.presentQueue
		);

		clz::log::info("created logical device");
		return true;
	}
} // namespace clz::renderer

// Destroyers
namespace clz::renderer
{
	void destroyDeviceContext()
	{
		destroyDevice();
		destroySurface();
		destroyDebugMessenger();
		destroyInstance();

		clz::log::info("Destroyed Device Context Successfully");
	}

	void destroyInstance()
	{
		vkDestroyInstance(r_deviceContext.instance, nullptr);
		clz::log::info("Destroyed vulkan instance");
	}

	void destroyDebugMessenger()
	{
		const auto destroyMessenger =
			reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
				r_deviceContext.instance,
				"vkDestroyDebugUtilsMessengerEXT"
			));
		if (destroyMessenger != nullptr)
		{
			destroyMessenger(
				r_deviceContext.instance,
				r_deviceContext.debugMessenger,
				nullptr
			);
		}
		clz::log::info("Destroyed debug messenger");
	}

	void destroySurface()
	{
		vkDestroySurfaceKHR(r_deviceContext.instance, r_deviceContext.surface, nullptr);
		clz::log::info("Destroyed surface instance");
	}

	void destroyDevice()
	{
		vkDestroyDevice(r_deviceContext.device, nullptr);
		clz::log::info("Destroyed logical device");
	}
} // namespace clz::renderer
