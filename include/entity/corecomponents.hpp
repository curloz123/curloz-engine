/**
 * @file corecomponents.hpp
 * @author curl0z
 * @brief ECS core components definitions
 */

#pragma once

#include "math/angle.hpp"
#include "math/quat.hpp"
#include "math/quateulerconv.hpp"
#include "math/vec3.hpp"
#include <cstdint>

namespace clz::ecs
{
	/// Dirty bits for the whole system
	enum class DirtyFlagBit : uint64_t
	{
		RENDERER = 1u << 0
	};

	/**
	 * @brief Stores position, rotation and scale of an entity in world space.
	 *
	 * Rotation is stored as quaternion
	 * This is the handshake between physics, renderer.
	 *
	 * @warning Do not change the layout without consulting the renderer and physics
	 * owners.
	 */
	class TransformComponent
	{
	private:
		math::quat m_rotation = {1, 0, 0, 0};
		math::vec3 m_position = {0, 0, 0};
		math::vec3 m_scale = {1, 1, 1};

		/// dirty flag
		uint64_t m_dirty = 0xFFFFFFFFFFFFFFFF;

		void markDirty()
		{
			m_dirty = 0xFFFFFFFFFFFFFFFF;
		}

	public:

		TransformComponent() = default;

		TransformComponent(
			const math::quat& rotation,
			const math::vec3& position,
			const math::vec3& scale
		):
			m_rotation(rotation),
			m_position(position),
			m_scale(scale),
			m_dirty(0xFFFFFFFFFFFFFFFF)
		{
		}

		const math::quat& getRotation() const
		{
			return m_rotation;
		}
		const math::vec3& getPosition() const
		{
			return m_position;
		}
		const math::vec3& getScale() const
		{
			return m_scale;
		}

		bool isDirty(const DirtyFlagBit dirtyFlagBit) const
		{
			return (m_dirty & static_cast<uint64_t>(dirtyFlagBit)) != 0;
		}

		void setRotation(const math::quat& rotation)
		{
			m_rotation = rotation;
			markDirty();
		}

		void setPosition(const math::vec3& position)
		{
			m_position = position;
			markDirty();
		}

		void setScale(const math::vec3& scale)
		{
			m_scale = scale;
			markDirty();
		}

		void unMarkDirty(const DirtyFlagBit dirtyFlagBit)
		{
			m_dirty &= ~(static_cast<uint64_t>(dirtyFlagBit));
		}
	};
	/**
	 * @brief Stores transform component as set by the editor.
	 *  This is necessary because after physics engine has calculated
	 *  and changed all the transform components **In Game Mode**, and
	 *  we go back to **Editor Mode**, we don't want the new physically
	 *  calculated transform do we??
	 *
	 *  @note Can only be changed by the editor, and initialized by scene at startup.
	 *  physics and rendering engine can only read from it.
	 *  No other subsystem is allowed to access it.
	 */
	struct EditorTransformComponent
	{
	private:
		math::vec3 m_rotation = {0, 0, 0};
		math::vec3 m_position = {0, 0, 0};
		math::vec3 m_scale = {1, 1, 1};
	public:

		EditorTransformComponent()
		{
		}

		EditorTransformComponent(
			const math::vec3& rotation,
			const math::vec3& position,
			const math::vec3& scale
		):
			m_rotation(rotation),
			m_position(position),
			m_scale(scale)
		{
		}

		EditorTransformComponent(const TransformComponent& transformComponent) :
			m_rotation(
				math::degrees(
					math::quatToEulerXYZ(
						transformComponent.getRotation()
					)
				)
			),
		     	m_position(transformComponent.getPosition()),
			m_scale(transformComponent.getScale())
		{
		}

		const math::vec3& getRotation() const
		{
			return m_rotation;
		}
		const math::vec3& getPosition() const
		{
			return m_position;
		}
		const math::vec3& getScale() const
		{
			return m_scale;
		}

		void setRotation(const math::vec3& rotation)
		{
			m_rotation = rotation;
		}
		void setPosition(const math::vec3& position)
		{
			m_position = position;
		}
		void setScale(const math::vec3& scale)
		{
			m_scale = scale;
		}
	};

} // namespace clz::ecs
