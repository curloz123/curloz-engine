/**
 * @file editor_types.hpp
 * @author curl0z
 * @brief Stores editor's global variables.
 */
#pragma once

#include "scene/entity/entitymanager.hpp"
#include <imgui.h>
#include <optional>
#include <vulkan/vulkan.h>

namespace clz::editor
{
	enum class EditorMode
	{
		Normal,
		PhysicsBodyEditor
	};

	inline EditorMode editorMode = EditorMode::Normal;

	/// @brief ImGui's descriptor pool
	inline VkDescriptorPool editorDescriptorPool;

	/// @brief In player window, current selected entity
	inline std::optional<ecs::entity> currentSelectedEntity;

	inline bool isEditorUsingRenderer = false;

	/// @brief Fonts
	inline ImFont* fontSans;
	inline ImFont* fontSansBold;
	inline ImFont* fontMono;
	inline ImFont* fontMonoBold;

} // namespace clz::editor