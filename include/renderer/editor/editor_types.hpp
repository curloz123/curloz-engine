#pragma once

#include <vulkan/vulkan.h>
#include <imgui.h>

namespace clz::editor
{
	inline VkDescriptorPool editorDescriptorPool;

	inline ImFont* fontSans;
	inline ImFont* fontSansBold;
	inline ImFont* fontMono;
	inline ImFont* fontMonoBold;
}