#pragma once

#include "scene/entity/entitymanager.hpp"
#include <imgui.h>
#include <optional>
#include <vulkan/vulkan.h>

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
} // namespace clz::editor