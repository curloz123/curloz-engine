/**
 * @file sensor_events.hpp
 * @author curl0z
 * @brief Manages all sensor events in physics system
 */

namespace clz::physics
{
	/**
	 * @brief Poll and process all pending sensor events from the physics world.
	 *
	 * Retrieves the current frame's sensor begin/end touch events from Box3D,
	 * resolves each involved shape's body to its attached ECS entity, and
	 * dispatches/logs the corresponding sensor event.
	 *
	 * @note To be ONLY called once per physics step
	 */
	void processSensorEvents();
}
