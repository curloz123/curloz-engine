/**
 * @file editorshortcuts.hpp
 * @author curl0z
 * @brief Maintains all editor shortcuts.
 * For example if foo key is pressed,
 * perform bar operation.
 */
#pragma once

#include "inspector/inspector.hpp"
#include "window/inputmanager.hpp"

namespace clz::editor
{
	/// @brief Polls for input, and checks if any shortcut is registered
	inline void processShortcuts()
	{
		if (window::isKeyPressed(input::Key::T))
		{
			ActiveTransform = TransformType::TRANSLATE;
		}
		if (window::isKeyPressed(input::Key::R))
		{
			ActiveTransform = TransformType::ROTATE;
		}
		if (window::isKeyPressed(input::Key::S))
		{
			ActiveTransform = TransformType::SCALE;
		}
	}

} // namespace clz::editor