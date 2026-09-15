#pragma once

#include "core/logs.hpp"
#include "window/inputmanager.hpp"
#include <deque>
#include <source_location>
#include <functional>

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

	inline void performUndo()
	{
		if (undoStack.empty())
			return;

		undoStack.back().undo();
		if (redoStack.size() >= MAX_SNAPSHOTS)
		{
			redoStack.pop_front();
		}
		redoStack.emplace_back(std::move(undoStack.back()));

		undoStack.pop_back();
	}
	inline void performRedo()
	{
		if (redoStack.empty())
			return;

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


	inline void clearSnapshots(const std::source_location& loc = std::source_location::current())
	{
		clz::log::info("snapshot cleared by: " + std::string(loc.function_name()));

		undoStack.clear();
		redoStack.clear();
	}
} // namespace clz::timemachine
