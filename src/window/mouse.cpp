#include "window/mouse.hpp"
#include "window/window_types.hpp"

namespace clz::window
{
	/// @copydoc
	void cursorCallback(GLFWwindow* window, const double xPos, const double yPos)
	{
		cursorPosThisFrame = {static_cast<float>(xPos), static_cast<float>(yPos)};
	}

	/// @copydoc
	void scrollCallback(GLFWwindow* window, const double xOffset, const double yOffset)
	{
		w_scrollOffset = static_cast<float>(yOffset);
	}
} // namespace clz::window