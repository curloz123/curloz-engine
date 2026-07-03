/**
 * @file inspector.hpp
 * @author curl0z
 * @brief Inspector window's functions
 * Inspector window shows all components
 * of current selected entity
 */

#pragma once

namespace clz::editor
{
	/**
	 * @brief Renders the Inspector panel for the currently selected entity.
	 */
	void showInspector();

	enum class TransformType
	{
		TRANSLATE,
		ROTATE,
		SCALE,
	};
	inline TransformType ActiveTransform = TransformType::TRANSLATE;
} // namespace clz::editor