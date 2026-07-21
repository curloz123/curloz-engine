/**
 * @file camera.cpp
 * @author curl0z
 * @brief Camera's main implementation file
 */

#include "renderer/camera/camera.hpp"
#include "core/enginestate.hpp"
#include "core/logs.hpp"
#include "include/offscreen/offscreentarget.hpp"
#include "math/vec2.hpp"
#include "renderer/camera/cameradata.hpp"
#include "renderer/camera/camerafunctions.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"

namespace clz::renderer::camera
{
	void setActiveCamera(const CameraID id)
	{
#ifdef CLZ_ENABLE_EDITOR
		const auto previousActiveCamera = activeCamera;
		if (id == EditorCam && previousActiveCamera == GameCam)
		{
			Yaw[id] = Yaw[previousActiveCamera];
			Pitch[id] = Pitch[previousActiveCamera];
			Position[id] = Position[previousActiveCamera];
			localFront[id] = localFront[previousActiveCamera];
			localRight[id] = localRight[previousActiveCamera];
			LastX[id] = LastX[previousActiveCamera];
			LastY[id] = LastY[previousActiveCamera];
			Fov[id] = Fov[previousActiveCamera];
		}
#endif

		FirstTime[id] = true;
		updateProjectionMatrix();
		activeCamera = id;
	}

	bool initializeCameras()
	{
		if (!loadCamera("game", GameCam))
		{
			clz::log::error("Could not load game camera");
			return false;
		}

#ifdef CLZ_ENABLE_EDITOR
		if (!loadCamera("editor", EditorCam))
		{
			clz::log::error("Could not load editor camera");
			return false;
		}
		setActiveCamera(EditorCam);
#else
		setActiveCamera(GameCam);
#endif

		return true;
	}

	void update()
	{
#ifdef CLZ_ENABLE_EDITOR
		if (window::isKeyPressed(input::Key::Escape))
		{
			window::enableCursor();
			setActiveCamera(EditorCam);
			setEngineState(state::EngineState::Editor, "KEY->ESCAPE, mid render loop");
		}
		if (window::isKeyPressed(input::Key::LeftControl) && window::isKeyPressed(input::Key::G) &&
		    state::g_engineState == state::EngineState::Editor)
		{
			window::disableCursor();
			setActiveCamera(GameCam);
			setEngineState(state::EngineState::Game, "KEY->CTRL+G, mid render loop");
		}

		if (editor::isCurrentlyShowingOffscreenTargets())
		{
			return;
		}

#endif

		const math::vec2 cursorPos = window::getCursorPosition();
		const float scroll = window::getScrollOffset();
		const auto id = activeCamera;
		processKeyBoardInput(id);
		processMouseInput(id, cursorPos.x, cursorPos.y);
		processMouseScroll(id, scroll);
	}
} // namespace clz::renderer::camera