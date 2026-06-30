#pragma once

#include "cameradata.hpp"
#include "camerafunctions.hpp"

namespace clz::renderer::camera
{
	/// @brief Switches the active camera and resets its mouse-delta tracking.
	inline void setActiveCamera(const CameraID id)
	{
		activeCamera = id;
		FirstTime[id] = true;
	}

	/// @brief Loads game (and editor, if enabled) camera config at startup.
	/// @return true if all required cameras loaded successfully.
	inline bool initializeCameras()
	{
		if (!loadCamera("game", GameCam))
		{
			clz::log::error("Could not load game camera");
			return false;
		}
		setActiveCamera(GameCam);

#ifdef CLZ_SHOW_EDITOR
		if (!loadCamera("editor", EditorCam))
		{
			clz::log::error("Could not load editor camera");
			return false;
		}
		setActiveCamera(EditorCam);
#endif

		return true;
	}

	/// @brief Drives input handling for the currently active camera.
	inline void update(const float xPos, const float yPos, const float scroll)
	{
		const auto id = activeCamera;
		processKeyBoardInput(id);
		processMouseInput(id, xPos, yPos);
		processMouseScroll(id, scroll);
	}

	/// @brief Returns the active camera's world-space position.
	inline math::vec3 getPosition()
	{
		return Position[activeCamera];
	}

	/// @brief Returns a point along the active camera's view direction, for lookAt targets.
	inline math::vec3 getTarget()
	{
		return Position[activeCamera] + localFront[activeCamera];
	}

	/// @brief Returns the active camera's field of view, in degrees.
	inline float getFov()
	{
		return Fov[activeCamera];
	}
}