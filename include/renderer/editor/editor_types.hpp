#pragma once

#include <vulkan/vulkan.h>
#include <imgui.h>
#include "ecs/entitymanager.hpp"
#include <optional>

namespace clz::editor
{
	/// @brief ImGui's descriptor pool
	inline VkDescriptorPool editorDescriptorPool;

	/// @brief In player window, current selected entity
	inline std::optional<ecs::entity> currentSelectedEntity;

	/// @brief Fonts
	inline ImFont* fontSans;
	inline ImFont* fontSansBold;
	inline ImFont* fontMono;
	inline ImFont* fontMonoBold;
}