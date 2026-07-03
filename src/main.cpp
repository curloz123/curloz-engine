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
#include "renderer/renderer.hpp"
#include "scene/scene.hpp"
#include "window/window.hpp"

int main()
{
	// Initialize config first. All subsystems depend on it
	clz::config::init();
	if (clz::log::errorOccurred()) [[unlikely]]
		return 1;
	// Print App details
	clz::log::info("Welcome to " + clz::config::getAppName());
	clz::config::printAppVersion();

	// Start clock, Whole system uses it, so make sure to start it first
	clz::time::init();

	// Initialize Window. Should be the first subsystem to initialize
	if (!clz::window::init()) [[unlikely]]
		return 1;

	// Initialize renderer
	if (!clz::renderer::init()) [[unlikely]]
		return 1;

	// Initialize audio
	clz::audio::init();

	// Initialize Scene
	clz::scene::loadScene();
	if (clz::log::errorOccurred()) [[unlikely]]
		return 1;


	// Main loop. Runs until g_engineState is set to EngineState::Shutdown
	while (clz::state::g_engineState != clz::state::EngineState::Shutdown)
	{
		clz::time::computeTime();
		clz::window::update();
		clz::renderer::update();
	}

	// Shut down
	clz::scene::saveScene();
	clz::audio::shutdown();
	clz::renderer::shutdown();
	clz::window::shutdown();
	clz::log::info("Exiting successfully");
	return 0;
}
