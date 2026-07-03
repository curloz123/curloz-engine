#include "core/JobSystem.h"

namespace curloz {
namespace core {

JobSystem::JobSystem() 
    : isTerminating(false)
    , activeJobs(0)
{
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) {
        numThreads = 1;
    }

    for (unsigned int i = 0; i < numThreads; ++i) {
        workerThreads.emplace_back(&JobSystem::WorkerLoop, this);
    }
}

JobSystem::~JobSystem() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        isTerminating = true;
    }
    
    condition.notify_all();

    for (std::thread& worker : workerThreads) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void JobSystem::Dispatch(std::function<void()> job) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        jobQueue.push(std::move(job));
    }
    activeJobs.fetch_add(1, std::memory_order_relaxed);
    condition.notify_one();
}

void JobSystem::Wait() {
    std::unique_lock<std::mutex> lock(queueMutex);
    waitCondition.wait(lock, [this]() {
        return activeJobs.load(std::memory_order_relaxed) == 0;
    });
}

void JobSystem::WorkerLoop() {
    while (true) {
        std::function<void()> job;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            
            condition.wait(lock, [this]() {
                return !jobQueue.empty() || isTerminating;
            });
            
            if (isTerminating && jobQueue.empty()) {
                return;
            }
            
            job = std::move(jobQueue.front());
            jobQueue.pop();
        }
        
        if (job) {
            job();
        }
        
        activeJobs.fetch_sub(1, std::memory_order_relaxed);
        waitCondition.notify_all();
    }
}

} // namespace core
} // namespace curloz
