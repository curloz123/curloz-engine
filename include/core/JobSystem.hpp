/**
 * @file JobSystem.hpp
 * @author Aditya Kumar Mishra
 * @brief Multithreaded job system providing a RAII thread pool for concurrent task execution.
 *
 * Implements a fixed-size thread pool whose lifetime is tied to the JobSystem
 * object (RAII). A lock-based task queue distributes work across workers.
 * Idle threads sleep on a condition variable, consuming near-zero CPU.
 * Jobs are dispatched as std::function<void()> and executed asynchronously
 * by the next available worker.
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace clz::jobs
{
	/**
	 * @brief Core thread pool that manages worker threads and a shared job queue.
	 *
	 * Lifecycle is fully RAII: the constructor spawns workers, the destructor
	 * signals termination and joins every thread. Any jobs remaining in the
	 * queue at shutdown are drained before threads exit.
	 *
	 * @note Thread-safe. Multiple threads may call dispatch() concurrently.
	 * @warning Do not destroy the JobSystem while external code still holds
	 *          references to it; call wait() first to ensure all work is done.
	 */
	class JobSystem
	{
	public:
		/**
		 * @brief Constructs the pool and spawns worker threads.
		 *
		 * Thread count defaults to std::thread::hardware_concurrency().
		 * Falls back to 1 if the query returns 0.
		 */
		JobSystem();

		/**
		 * @brief Signals all workers to stop and joins every thread.
		 *
		 * Remaining queued jobs are executed before threads exit.
		 */
		~JobSystem();

		/// @brief Deleted — the pool owns its threads exclusively.
		JobSystem(const JobSystem&) = delete;

		/// @brief Deleted — the pool owns its threads exclusively.
		JobSystem& operator=(const JobSystem&) = delete;

		/**
		 * @brief Enqueues a callable for asynchronous execution.
		 *
		 * The job is pushed onto the thread-safe queue and a sleeping
		 * worker is woken via notify_one().
		 *
		 * @param job Callable (lambda / std::function) to run on a worker.
		 */
		void dispatch(std::function<void()> job);

		/**
		 * @brief Blocks the calling thread until every dispatched job has finished.
		 *
		 * Useful as a frame-boundary fence: dispatch a batch of work, then
		 * call wait() before moving to the next pipeline stage.
		 */
		void wait();

	private:
		std::vector<std::thread> js_workers;        ///< Pool of persistent worker threads
		std::queue<std::function<void()>> js_queue;  ///< Pending jobs awaiting execution

		std::mutex js_mutex;                         ///< Guards js_queue and js_activeJobs increments
		std::condition_variable js_wakeCond;          ///< Wakes a sleeping worker when a job arrives
		std::condition_variable js_waitCond;          ///< Notifies wait() callers on job completion

		std::atomic<bool> js_terminating;             ///< True once the destructor begins teardown
		std::atomic<uint32_t> js_activeJobs;          ///< Count of queued + currently executing jobs

		/**
		 * @brief Main loop executed by each worker thread.
		 *
		 * Sleeps on js_wakeCond until a job is available or termination
		 * is signalled. Decrements js_activeJobs after each job completes
		 * and notifies js_waitCond so wait() can return.
		 */
		void workerLoop();
	};

} // namespace clz::jobs
