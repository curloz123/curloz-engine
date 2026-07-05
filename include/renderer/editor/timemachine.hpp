/**
 * @file timemachine.hpp
 * @author curl0z
 * @brief Undo/redo system for the editor, based on a type-erased snapshot stack.
 *
 * Workflow:
 *  1. An edit begins (slider activated / gizmo drag started etc.) -> caller captures
 *     the "before" state of the component.
 *  2. The edit ends (slider deactivated after edit / gizmo drag ended) -> caller
 *     calls createSnapshot<T>(entity, oldState, newState).
 *  3. createSnapshot() pushes a type-erased Snapshot<T> onto the stack. If a new
 *     action is created while snapshotPointer isn't at the newest entry (i.e. the
 *     user undid some actions first), every entry after snapshotPointer is purged
 *     before pushing — this discards the old redo branch, matching standard
 *     editor undo/redo semantics.
 *  4. Ctrl+Z / Ctrl+Shift+Z call undo()/redo(), which apply the stored old/new
 *     state via virtual dispatch, without needing to know the concrete type T.
 */

#pragma once

#include "scene/entity/componentmanager.hpp"
#include "scene/entity/entitymanager.hpp"
#include "window/inputmanager.hpp"
#include <memory>

namespace clz::snapshot
{
	/// @brief Maximum number of snapshots retained; oldest is evicted once exceeded.
	constexpr uint8_t MAX_SNAPSHOTS = 30;

	/// @brief Type-erased base for a single undoable/redoable change.
	struct ISnapshotBase
	{
		virtual ~ISnapshotBase() = default;

		/// @brief Reverts the entity's component to its pre-edit state.
		virtual void undo(const ecs::entity e) = 0;

		/// @brief Reapplies the entity's component to its post-edit state.
		virtual void redo(const ecs::entity e) = 0;
	};

	/// @brief Concrete snapshot holding the before/after value of one component type.
	template <typename T> struct Snapshot : ISnapshotBase
	{
		T oldSnapshot;
		T newSnapshot;

		void undo(const ecs::entity e) override
		{
			ecs::setComponent<T>(e, oldSnapshot);
		}
		void redo(const ecs::entity e) override
		{
			ecs::setComponent<T>(e, newSnapshot);
		}
	};

	/// @brief Index of the last applied snapshot. -1 means nothing has been applied yet.
	inline int8_t snapshotPointer = -1;

	/// @brief Entity each snapshot at the matching index belongs to. Kept in lockstep with Snapshots.
	inline std::vector<ecs::entity> SnapshottedEntity;

	/// @brief The undo/redo stack itself.
	inline std::vector<ISnapshotBase*> Snapshots{};

	/// @brief Edge-detection state for the Z key, since isKeyPressed is level-triggered.
	inline bool ZPressedLastFrame = false;
	inline bool ZPressedThisFrame = false;

	/// @brief Reverts to the previous snapshot's old state, if any remain.
	inline void undo()
	{
		if (snapshotPointer < 0)
			return;

		Snapshots[snapshotPointer]->undo(SnapshottedEntity[snapshotPointer]);

		--snapshotPointer;
		if (snapshotPointer < -1)
			snapshotPointer = -1;
	}

	/// @brief Reapplies the next snapshot's new state, if any remain ahead of the pointer.
	inline void redo()
	{
		// Nothing to redo once we're already at (or past) the newest entry.
		if (snapshotPointer >= static_cast<int>(Snapshots.size()) - 1)
			return;

		if (snapshotPointer < 0)
			snapshotPointer = 0;
		else
			++snapshotPointer;

		Snapshots[snapshotPointer]->redo(SnapshottedEntity[snapshotPointer]);
	}
} // namespace clz::snapshot

namespace clz::editor
{
	/// @brief Records a completed edit as a new undo/redo entry.
	///
	/// If the stack is full, the oldest entry is evicted. If new activity happens
	/// while the pointer isn't at the newest entry (i.e. after an undo), every
	/// entry ahead of the pointer is discarded first, collapsing the old redo branch.
	template <typename T> void createSnapshot(const ecs::entity entity, const T& oldSnapshot, const T& newSnapshot)
	{
		if (snapshot::Snapshots.size() == snapshot::MAX_SNAPSHOTS)
		{
			delete snapshot::Snapshots.front();
			snapshot::Snapshots.erase(snapshot::Snapshots.begin());
			snapshot::SnapshottedEntity.erase(snapshot::SnapshottedEntity.begin());
		}

		if (snapshot::snapshotPointer < static_cast<int>(snapshot::Snapshots.size()) - 1)
		{
			for (int i = static_cast<int>(snapshot::Snapshots.size()) - 1; i > snapshot::snapshotPointer; --i)
			{
				delete snapshot::Snapshots[i];
				snapshot::Snapshots.pop_back();
				snapshot::SnapshottedEntity.pop_back();
			}
		}

		snapshot::Snapshot<T>* snapshot = new snapshot::Snapshot<T>;
		snapshot->oldSnapshot = oldSnapshot;
		snapshot->newSnapshot = newSnapshot;
		snapshot::Snapshots.push_back(snapshot);
		snapshot::SnapshottedEntity.push_back(entity);
		snapshot::snapshotPointer = static_cast<int8_t>(snapshot::Snapshots.size() - 1);
	}

	/// @brief Polls Ctrl+Z / Ctrl+Shift+Z with edge detection and dispatches undo/redo.
	inline void timeTravel()
	{
		if (snapshot::Snapshots.empty())
			return;

		snapshot::ZPressedThisFrame = window::isKeyPressed(input::Key::Z);
		const bool leftShiftPressed = window::isKeyPressed(input::Key::LeftShift);
		const bool leftControlPressed = window::isKeyPressed(input::Key::LeftControl);

		if (leftControlPressed && snapshot::ZPressedThisFrame && !snapshot::ZPressedLastFrame)
		{
			if (leftShiftPressed)
				snapshot::redo();
			else
				snapshot::undo();
		}

		snapshot::ZPressedLastFrame = snapshot::ZPressedThisFrame;
	}
} // namespace clz::editor