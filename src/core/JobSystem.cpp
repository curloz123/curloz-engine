/**
 * @file JobSystem.cpp
 * @author Aditya Kumar Mishra
 * @brief Implementation of the clz::jobs::JobSystem thread pool.
 */

#include "core/JobSystem.hpp"

namespace clz::jobs
{

	JobSystem::JobSystem()
		: js_terminating(false)
		, js_activeJobs(0)
	{
		unsigned int count = std::thread::hardware_concurrency();
		if (count == 0)
		{
			count = 1;
		}

		for (unsigned int i = 0; i < count; ++i)
		{
			js_workers.emplace_back(&JobSystem::workerLoop, this);
		}
	}

	JobSystem::~JobSystem()
	{
		{
			std::unique_lock<std::mutex> lock(js_mutex);
			js_terminating.store(true, std::memory_order_relaxed);
		}

		js_wakeCond.notify_all();

		for (std::thread& w : js_workers)
		{
			if (w.joinable())
			{
				w.join();
			}
		}
	}

	void JobSystem::dispatch(std::function<void()> job)
	{
		{
			std::unique_lock<std::mutex> lock(js_mutex);
			js_queue.push(std::move(job));
			js_activeJobs.fetch_add(1, std::memory_order_relaxed);
		}
		js_wakeCond.notify_one();
	}

	void JobSystem::wait()
	{
		std::unique_lock<std::mutex> lock(js_mutex);
		js_waitCond.wait(lock, [this]() {
			return js_activeJobs.load(std::memory_order_acquire) == 0;
		});
	}

	void JobSystem::workerLoop()
	{
		while (true)
		{
			std::function<void()> job;

			{
				std::unique_lock<std::mutex> lock(js_mutex);

				js_wakeCond.wait(lock, [this]() {
					return !js_queue.empty() || js_terminating.load(std::memory_order_relaxed);
				});

				if (js_terminating.load(std::memory_order_relaxed) && js_queue.empty())
				{
					return;
				}

				job = std::move(js_queue.front());
				js_queue.pop();
			}

			if (job)
			{
				job();
			}

			js_activeJobs.fetch_sub(1, std::memory_order_acq_rel);
			js_waitCond.notify_all();
		}
	}

} // namespace clz::jobs
