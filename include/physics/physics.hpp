/**
 * @file physics.hpp
 * @author curl0z
 * @brief Main Header file of physics subsystem.
 * Provides init, update, and shutdown as usual.
 */
#pragma once

#include <box3d/box3d.h>

namespace clz::physics
{
	/// @brief Initializes physics subsystem
	bool init();

	/// @brief Updates the physics subsystem
	void update();

	/// @brief Shuts down physics subsystem
	void shutdown();
} // namespace clz::physics

namespace clz::physics
{
	/// @brief typename identifiers for external subsystems to use
	using RigidBodyId = uint32_t;

	/// @brief Shape Id of any created body, is just index to shape's vector
	struct RigidBodyShapeId
	{
		uint8_t value;

		[[nodiscard]] bool operator==(const RigidBodyShapeId otherId)
		{
			return this->value == otherId.value;
		}
	};
} // namespace clz::physics
