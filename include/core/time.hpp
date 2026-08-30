/**
 * @file time.hpp
 * @author curl0z
 * @brief Engine time and delta time utilities.
 *
 * Provides frame timing via std::chrono and an optional software
 * frame cap when vsync is disabled.
 *
 * @note init() must be called after config::init() as it reads
 * vsync and refresh rate from the window config section.
 * @note computeTime must be the first funcion to be called at update loop
 */

#pragma once

#include "config/config.hpp"
#include "core/logs.hpp"
#include <chrono>
#include <thread>

namespace clz::time
{
	/// @brief Actual delta time
	inline float t_deltaTime = 0.0;

	/// @brief Total elapsed time
	inline double t_totalElapsedTime = 0.0;

	/// @brief Time at the start of engine
	inline std::chrono::time_point<std::chrono::steady_clock> t_timeAtStart;

	/// @brief Timestamp of the current frame.
	inline std::chrono::time_point<std::chrono::steady_clock> t_timeTakenThisFrame;

	/// @brief Timestamp of the previous frame.
	inline std::chrono::time_point<std::chrono::steady_clock> t_timeTakenLastFrame;

	/// @brief Whether vsync should be enabled.
	inline bool t_VSync = true;

	/// @brief Target refresh rate in Hz. Used to compute target delta time.
	inline uint8_t t_refreshRate = 60;

	/// @brief Target delta time in seconds derived from refresh rate. Only applied if
	/// vsync is enabled
	inline float t_targetDeltaTime = 1.0f / 60;

	/**
	 * @brief Initializes the time subsystem.
	 *
	 * Seeds both frame timestamps to now and reads vsync and
	 * refresh rate from the config subsystem.
	 *
	 * @note config::init() must be called before this.
	 */
	inline void init()
	{
		t_timeAtStart 	     = std::chrono::steady_clock::now();
		t_timeTakenLastFrame = std::chrono::steady_clock::now();
		t_timeTakenThisFrame = std::chrono::steady_clock::now();

		t_VSync 	     = clz::config::getValue<bool>("engine", "vsync", true);
		t_refreshRate        = config::getValue<int>("engine", "refreshrate", 60);
		t_targetDeltaTime    = 1.0f / static_cast<float>(t_refreshRate);

		clz::log::info("target dt: " + std::to_string(t_targetDeltaTime));
	}

	/**
	 * @brief Computes and returns delta time in seconds since the last frame.
	 *
	 * If vsync is enabled and the frame finished faster than the target
	 * delta, the function sleeps the remaining time to cap the frame rate.
	 *
	 */
	inline void computeTime()
	{
		t_timeTakenThisFrame 	= std::chrono::steady_clock::now();
		t_deltaTime 		= std::chrono::duration<float>(
						t_timeTakenThisFrame - t_timeTakenLastFrame).count();
		t_timeTakenLastFrame 	= t_timeTakenThisFrame;
		t_totalElapsedTime 	+= t_deltaTime;

		// Software frame cap — sleep remaining time if frame finished early
		if ((t_VSync) && (t_deltaTime < t_targetDeltaTime))
		{
			std::this_thread::sleep_for(
				std::chrono::duration<double>(t_targetDeltaTime - t_deltaTime)
			);
			t_deltaTime = t_targetDeltaTime;
		}
	}

	/**
	 * @brief Returns delta time
	 * @return t_deltaTime in seconds
	 */
	inline float getDeltaTime()
	{
		return t_deltaTime;
	}

	/**
	 * @brief Returns Total elapsed time
	 * @return total elapsed at this moment in seconds
	 */
	inline double getTotalElapsedTime()
	{
		const auto time = std::chrono::steady_clock::now();
		return std::chrono::duration<double>(time - t_timeAtStart).count();
	}

} // namespace clz::time
