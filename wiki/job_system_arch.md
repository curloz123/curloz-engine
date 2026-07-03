# Job System — Architecture & Workflow

## Overview

The Job System (`clz::jobs::JobSystem`) is a core multithreading subsystem for
the Curloz Engine. It manages a fixed-size RAII thread pool that distributes
work across all available CPU cores, reducing frame time and maximising
hardware utilisation. By reusing persistent worker threads, we avoid the
overhead of creating and destroying OS threads for every task.

## Architecture

### Components

| Component | Type | Purpose |
|---|---|---|
| `js_workers` | `std::vector<std::thread>` | Persistent pool of worker threads, sized to `hardware_concurrency()` |
| `js_queue` | `std::queue<std::function<void()>>` | FIFO queue holding pending jobs |
| `js_mutex` | `std::mutex` | Guards all access to `js_queue` and `js_activeJobs` increments |
| `js_wakeCond` | `std::condition_variable` | Wakes a sleeping worker when a new job is enqueued |
| `js_waitCond` | `std::condition_variable` | Notifies the caller of `wait()` when all jobs finish |
| `js_activeJobs` | `std::atomic<uint32_t>` | Tracks the number of queued + executing jobs |
| `js_terminating` | `std::atomic<bool>` | Signals workers to exit during destruction |

### RAII Lifecycle

- **Constructor:** Spawns `N` worker threads (`N = hardware_concurrency()`).
  Each thread enters `workerLoop()` and immediately sleeps on `js_wakeCond`.
- **Destructor:** Sets `js_terminating = true` under the lock, broadcasts
  `js_wakeCond.notify_all()`, and joins every thread. Workers drain any
  remaining queued jobs before exiting.

## Workflow

The lifecycle of a single job follows these steps:

```
 Caller              Queue               Worker Thread
   │                   │                       │
   │  dispatch(job)    │                       │  (sleeping on js_wakeCond)
   │──────────────────►│                       │
   │  lock mutex       │                       │
   │  push job         │                       │
   │  ++js_activeJobs  │                       │
   │  unlock mutex     │                       │
   │  notify_one()─────┼──────────────────────►│  (wakes up)
   │                   │                       │  lock mutex
   │                   │◄──────────────────────│  pop job
   │                   │                       │  unlock mutex
   │                   │                       │  execute job()
   │                   │                       │  --js_activeJobs
   │                   │                       │  notify_all(js_waitCond)
   │                   │                       │
   │  wait()           │                       │
   │  (blocks until    │                       │
   │   js_activeJobs   │                       │
   │   reaches 0)      │                       │
   │◄──────────────────┼───────────────────────│
```

1. **Dispatch:** The caller invokes `dispatch()` with a lambda or function.
   The mutex is acquired, the job is pushed onto `js_queue`, `js_activeJobs`
   is incremented (inside the lock to prevent a race with completion), the
   mutex is released, and `js_wakeCond.notify_one()` wakes a sleeping worker.

2. **Execution:** A worker wakes, re-acquires the mutex, dequeues the front
   job, releases the mutex, and executes the job outside the critical section.

3. **Completion:** After the job returns, the worker atomically decrements
   `js_activeJobs` and calls `js_waitCond.notify_all()` so any thread blocked
   in `wait()` can re-check the counter.

4. **Synchronisation:** `wait()` blocks on `js_waitCond` with a predicate
   that returns `true` only when `js_activeJobs == 0`. This acts as a
   frame-boundary fence.

## Integration Points

The Job System is designed to slot into the engine's existing subsystem
architecture. Key integration opportunities:

### 1. Asset Loading

Offload blocking file I/O, image decoding (PNG/JPG via stb_image), and mesh
parsing (OBJ/GLTF via Assimp) to the pool. The main thread dispatches load
requests and can continue rendering a loading screen without frame drops.

```cpp
clz::jobs::JobSystem pool;
pool.dispatch([&]() { loadTexture("assets/textures/diffuse.png"); });
pool.dispatch([&]() { loadModel("assets/models/scene.gltf"); });
pool.wait(); // block until all assets are ready
```

### 2. Physics Step

Distribute independent rigid body integration (position/velocity updates)
and broad-phase collision detection (AABB overlap, octree updates) across
cores. Each body or spatial partition can be processed as a separate job.

```cpp
for (auto& body : rigidbodies)
{
        pool.dispatch([&body, dt]() { body.integrate(dt); });
}
pool.wait(); // all bodies updated before narrow-phase
```

### 3. Render Command Generation

Modern graphics APIs (Vulkan, DirectX 12) support recording command buffers
in parallel. The Job System can dispatch tasks that build draw calls for
different segments of the scene graph, which are later submitted on the
main render thread.

```cpp
for (auto& chunk : sceneChunks)
{
        pool.dispatch([&chunk]() { chunk.recordDrawCommands(); });
}
pool.wait(); // all command buffers ready for submission
```
