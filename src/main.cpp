/**
 * @file main.cpp
 * @author curl0z
 * @brief Entry point for Curloz Engine.
 *
 * Initializes all subsystems in dependency order, runs the main loop,
 * and shuts everything down cleanly on exit.
 *
 * @note Subsystem init order matters — config must come first
 * as all other subsystems read from it.
 */

#include "audio/audio.hpp"
#include "config/config.hpp"
#include "core/enginestate.hpp"
#include "core/logs.hpp"
#include "core/time.hpp"
#include "entity/entity.hpp"
#include "physics/physics.hpp"
#include "renderer/renderer.hpp"
#include "scene/scene.hpp"
#include "window/window.hpp"

#ifdef CLZ_ENABLE_EDITOR
#include "../editor/include/editor.hpp"
#endif

int main()
{
	// Initialize config first. All subsystems depend on it
	if (!clz::config::init())
		return 1;
	clz::log::info("Welcome to " + clz::config::getValue<std::string>("engine", "name", "Curloz Engine"));
	clz::log::info("Version: " + 
		std::to_string(clz::config::getValue<int>("engine", "version_major", 0)) + "." + 
		std::to_string(clz::config::getValue<int>("engine", "version_minor", 0)) + "." +
		std::to_string(clz::config::getValue<int>("engine", "version_patch", 0))
	);

	// Start clock, Whole system uses it, so make sure to start it first
	clz::time::init();

	// Initialize Window. Should be the first subsystem to initialize
	if (!clz::window::init()) [[unlikely]]
		return 1;

	// Initialize physics
	if (!clz::physics::init()) [[unlikely]]
		return 1;

	// Initialize renderer
	if (!clz::renderer::init()) [[unlikely]]
		return 1;
#ifdef CLZ_ENABLE_EDITOR
	if (!clz::editor::init()) [[unlikely]]
		return 1;
#endif

	// Initialize audio
	clz::audio::init();

	// Initialize entity system
	clz::ecs::init();

	// Initialize Scene
	if (!clz::scene::loadScene()) [[unlikely]]
		return 1;

	// Main loop. Runs until g_engineState is set to EngineState::Shutdown
	while (clz::state::g_engineState != clz::state::EngineState::Shutdown)
	{
		clz::time::computeTime();
		clz::state::updateEngineState();
		clz::window::update();
		clz::physics::update();
		// editor is updated by renderer itself
		clz::renderer::update();
	}

	// Shut down
	clz::scene::saveScene();
	clz::ecs::shutdown();
	clz::audio::shutdown();
#ifdef CLZ_ENABLE_EDITOR
	clz::editor::shutdown();
#endif
	clz::renderer::shutdown();
	clz::physics::shutdown();
	clz::window::shutdown();

	clz::config::shutdown();
	clz::log::info("Exiting successfully");
	return 0;
}
