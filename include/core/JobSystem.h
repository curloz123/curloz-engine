#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <atomic>

namespace curloz {
namespace core {

/**
 * @class JobSystem
 * @brief A robust, multithreaded Job System for concurrent task execution.
 *
 * The JobSystem manages a thread pool that dynamically scales to the available
 * hardware concurrency. It utilizes a thread-safe task queue to distribute workloads
 * across worker threads.
 *
 * @note This class is thread-safe. Multiple threads can safely dispatch jobs or wait
 * on completion simultaneously.
 */
class JobSystem {
public:
    /**
     * @brief Constructs the JobSystem and initializes the thread pool.
     *
     * The number of spawned worker threads is determined by `std::thread::hardware_concurrency()`.
     * Workers will sleep until jobs are dispatched.
     */
    JobSystem();

    /**
     * @brief Destroys the JobSystem and safely terminates all worker threads.
     *
     * Sets the termination flag and joins all active threads to ensure a clean shutdown.
     */
    ~JobSystem();

    // Delete copy and move semantics to enforce unique ownership
    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    /**
     * @brief Dispatches a job to be executed asynchronously by a worker thread.
     *
     * Safely enqueues the provided lambda or function into the task queue and notifies
     * a sleeping worker thread to pick it up.
     *
     * @param job A `std::function<void()>` representing the task to execute.
     */
    void Dispatch(std::function<void()> job);

    /**
     * @brief Blocks the calling thread until all dispatched jobs have completed.
     *
     * This is useful for synchronization points, such as waiting for a batch of physics
     * jobs to finish before moving to the rendering phase of a frame.
     */
    void Wait();

private:
    std::vector<std::thread> workerThreads;          ///< The pool of worker threads.
    std::queue<std::function<void()>> jobQueue;      ///< Thread-safe queue containing pending jobs.

    std::mutex queueMutex;                           ///< Mutex ensuring thread-safe access to the job queue.
    std::condition_variable condition;               ///< Condition variable used to wake sleeping worker threads.
    std::condition_variable waitCondition;           ///< Condition variable used to block the Wait() caller.

    std::atomic<bool> isTerminating;                 ///< Flag indicating if the system is shutting down.
    std::atomic<uint32_t> activeJobs;                ///< Atomic counter tracking the number of pending and executing jobs.

    /**
     * @brief The core execution loop run by each worker thread.
     *
     * Continuously attempts to dequeue and execute jobs. If the queue is empty,
     * the thread will yield and sleep on `condition` until a new job is dispatched.
     */
    void WorkerLoop();
};

} // namespace core
} // namespace curloz
