/**
 * @file physics.hpp
 * @author curl0z
 * @brief Main Header file of physics subsystem.
 * Provides init, update, and shutdown as usual.
 */
#pragma once

namespace clz::physics
{
	/// @brief Initializes physics subsystem
	bool init();

	/**
	 * @brief Updates the physics subsystem
	 * Basically transforms all rigid bodies registered
	 */
	void update();

	/// @brief Shuts down physics subsystem
	void shutdown();
} // namespace clz::physics