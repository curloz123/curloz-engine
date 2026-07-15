/**
 * @file physics_types.hpp
 * @author curl0z
 * @brief Defines all global variables to be used
 * within the physics subsystem
 */
#pragma once

#include <box3d/box3d.h>

namespace clz::physics
{
	/// @brief Global physics world
	inline b3WorldId p_world = b3_nullWorldId;
	/// @brief Gravity
	inline b3Vec3 p_gravity = {0.0, -10.0f, 0.0f};
	/// @brief Time step
	inline float p_timeStep = 1.0f/60.0f;
	/// @brief Sub step count
	inline int p_subStepCount = 4;
	/// @brief Accumulator
	inline float p_accumulator = 0.0f;
}

