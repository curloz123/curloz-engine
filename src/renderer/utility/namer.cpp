#include "renderer/utility/namer.hpp"
#include "core/assert.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	static PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT = nullptr;

	void setHandleName(const uint64_t handle, const VkObjectType objectType, const char* name)
	{
#ifdef CLZ_DEBUG
		// 1. If we haven't fetched the address yet, look it up using the active
		// VkInstance. NOTE: Since this function only passes the device, we need
		// access to the instance pointer. Swap 'r_instanceContext.instance' below
		// with whatever your global instance variable is named.
		if (!pfnSetDebugUtilsObjectNameEXT)
		{
			pfnSetDebugUtilsObjectNameEXT =
				reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
					vkGetInstanceProcAddr(
						r_deviceContext.instance,
						"vkSetDebugUtilsObjectNameEXT"
					)
				);
		}
		CLZ_ASSERT(
			pfnSetDebugUtilsObjectNameEXT,
			"could not load vulkan's object naming function"
		);

		if (pfnSetDebugUtilsObjectNameEXT)
		{
			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.pNext = nullptr;
			nameInfo.objectType = objectType;
			nameInfo.objectHandle = handle;
			nameInfo.pObjectName = name;

			// r_deviceContext.device is already being used successfully in your
			// other files
			pfnSetDebugUtilsObjectNameEXT(
				clz::renderer::r_deviceContext.device,
				&nameInfo
			);
		}

#endif
	}
} // namespace clz::renderer