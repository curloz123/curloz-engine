/**
 * @file inspector.hpp
 * @author curl0z
 * @brief Inspector window's functions
 * Inspector window shows all components
 * of current selected entity
 *
 * Also Saves a snapshot upon every change made
 */

#pragma once

#include <vulkan/vulkan.h>

namespace clz::editor
{
	/**
	 * @brief Renders the Inspector panel for the currently selected entity.
	 */
	void showInspector(VkCommandBuffer commandBuffer);

	enum class TransformType
	{
		TRANSLATE,
		ROTATE,
		SCALE,
	};
	inline TransformType ActiveTransform = TransformType::TRANSLATE;
} // namespace clz::editor