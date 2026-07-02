#pragma once

#include "inspector.hpp"
#include "window/inputmanager.hpp"

namespace clz::editor
{
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

}