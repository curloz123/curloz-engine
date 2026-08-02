#pragma once

#include "core/logs.hpp"
#include "window/inputmanager.hpp"
#include <deque>
#include <source_location>

namespace clz::timemachine
{
constexpr uint8_t MAX_SNAPSHOTS = 50;

struct Snapshot
{
	std::function<void()> undo;
	std::function<void()> redo;
};
inline std::deque<Snapshot> undoStack;
inline std::deque<Snapshot> redoStack;

inline void undo()
{
	if (undoStack.empty())
		return;

	clz::log::debug("going behind");
	undoStack.back().undo();
	if (redoStack.size() >= MAX_SNAPSHOTS)
	{
		redoStack.pop_front();
	}
	redoStack.emplace_back(std::move(undoStack.back()));

	undoStack.pop_back();
}
inline void redo()
{
	if (redoStack.empty())
		return;

	clz::log::debug("going ahead");
	redoStack.back().redo();
	if (undoStack.size() >= MAX_SNAPSHOTS)
	{
		undoStack.pop_front();
	}
	undoStack.emplace_back(std::move(redoStack.back()));

	redoStack.pop_back();
}

inline void createSnapshot(std::function<void()> undo, std::function<void()> redo)
{
	if (undoStack.size() >= MAX_SNAPSHOTS)
		undoStack.pop_front();
	if (!redoStack.empty())
		redoStack.clear();

	undoStack.emplace_back(Snapshot{.undo = std::move(undo), .redo = std::move(redo)});
}

inline bool ZPressedLastFrame = false;
inline bool ZPressedThisFrame = false;
inline void timeTravel()
{
	if (undoStack.empty() && redoStack.empty())
	{
		return;
	}

	ZPressedThisFrame = window::isKeyPressed(input::Key::Z);
	const bool leftShiftPressed = window::isKeyPressed(input::Key::LeftShift);
	const bool leftControlPressed = window::isKeyPressed(input::Key::LeftControl);

	if (leftControlPressed && ZPressedThisFrame && !ZPressedLastFrame)
	{
		if (leftShiftPressed)
		{
			redo();
		}
		else
		{
			undo();
		}
	}

	ZPressedLastFrame = ZPressedThisFrame;
}

inline void clearSnapshots(const std::source_location& loc = std::source_location::current())
{
	clz::log::debug("snapshot created by: ", loc);

	undoStack.clear();
	redoStack.clear();
}
} // namespace clz::timemachine