/**
 * @file descriptorlayout.hpp
 * @author curl0z
 * @brief Descriptor set layout creation utilities.
 */
#pragma once

namespace clz::renderer
{
	/**
	 * @brief Creates the main descriptor set layout.
	 * @return True on success.
	 */
	bool createDescriptorSetLayout();

	/**
	 * @brief Destroys the descriptor set layout.
	 */
	void destroyDescriptorSetLayout();
} // namespace clz::renderer