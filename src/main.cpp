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
#include "ecs/ecs.hpp"
#include "renderer/renderer.hpp"
#include "scripting/scripting.hpp"
#include "window/window.hpp"
#include "window/mouse.hpp"

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
	clz::window::init();
	if (clz::log::errorOccurred()) [[unlikely]]
		return 1;



	// Initialize renderer
	if (!clz::renderer::init()) [[unlikely]]
		return 1;



	// Initialize audio
	clz::audio::init();



	// Initialize entities
	clz::ecs::init();
	if (clz::log::errorOccurred()) [[unlikely]]
		return 1;



	// Initialize script at last
	clz::script::init();
	clz::script::runScript("assets/scripts/test.lua");


	// TEST
	clz::window::disableCursor();

	// Main loop. Runs until g_engineState is set to EngineState::Shutdown
	while (clz::state::g_engineState != clz::state::EngineState::Shutdown)
	{
		clz::time::computeTime();
		clz::window::update();
		clz::renderer::update();
	}

	// Shut down
	clz::audio::shutdown();
	clz::ecs::shutdown();
	clz::renderer::shutdown();
	clz::window::shutdown();
	clz::log::info("Exiting successfully");
	return 0;
}
