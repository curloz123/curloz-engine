/**
 * @file JobSystemBenchmark.cpp
 * @author Aditya Kumar Mishra
 * @brief Standalone stress test for clz::jobs::JobSystem.
 *
 * Dispatches 10 000 compute-bound jobs across the thread pool, measures
 * wall-clock time via std::chrono, and compares against a single-threaded
 * sequential baseline to demonstrate the speedup.
 *
 * Build:  cmake --build build --target JobSystemBenchmark
 * Run:    ./build/JobSystemBenchmark
 */

#include "core/JobSystem.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <print>
#include <thread>

/// @brief Simulates a small unit of compute-bound work (sin loop).
static void doWork()
{
	volatile double result = 0.0;
	for (int i = 0; i < 10000; ++i)
	{
		result += std::sin(static_cast<double>(i));
	}
}

int main()
{
	constexpr uint32_t js_totalJobs = 10000;
	unsigned int js_threadCount = std::thread::hardware_concurrency();

	std::println("=== Job System Stress Test ===");
	std::println("Jobs to dispatch : {}", js_totalJobs);
	std::println("Hardware threads : {}", js_threadCount);
	std::println("");

	// --- Single-threaded baseline ---
	std::println("Running single-threaded baseline...");
	auto stStart = std::chrono::high_resolution_clock::now();
	for (uint32_t i = 0; i < js_totalJobs; ++i)
	{
		doWork();
	}
	auto stEnd = std::chrono::high_resolution_clock::now();
	auto stMs = std::chrono::duration_cast<std::chrono::milliseconds>(stEnd - stStart).count();

	// --- Multi-threaded via JobSystem ---
	std::println("Running multi-threaded job system...");
	clz::jobs::JobSystem pool;

	auto mtStart = std::chrono::high_resolution_clock::now();
	for (uint32_t i = 0; i < js_totalJobs; ++i)
	{
		pool.dispatch(doWork);
	}
	pool.wait();
	auto mtEnd = std::chrono::high_resolution_clock::now();
	auto mtMs = std::chrono::duration_cast<std::chrono::milliseconds>(mtEnd - mtStart).count();

	// --- Results ---
	double speedup = (mtMs > 0) ? static_cast<double>(stMs) / static_cast<double>(mtMs) : 0.0;

	std::println("");
	std::println("--- Results ---");
	std::println("Single-threaded  : {} ms", stMs);
	std::println("Multi-threaded   : {} ms", mtMs);
	std::println("Speedup          : {:.2f}x", speedup);

	return 0;
}
