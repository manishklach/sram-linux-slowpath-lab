# Experimental Results

This document summarizes the findings from the SRAM Linux Slowpath Lab benchmarks.

## Finding 1: Median Latency Collapse
By transitioning from per-request memory setup and interrupt-based completions to pre-registered buffers and polling, we observe a significant reduction in median (p50) latency. 
- **Baseline**: ~40µs
- **Polling**: ~21µs
This represents a ~47% reduction in end-to-end latency, proving that host overhead accounts for nearly half of the total round-trip time when device execution is extremely fast (20µs).

## Finding 2: Tail Latency Persists
Despite optimizations, p99 and p999 latencies remain significantly higher than the median. This is largely due to the underlying OS scheduler and hypervisor (WSL/Windows) pre-empting the benchmark thread. 
- **p999**: Often exceeds 150µs-200µs even in Polling mode.
This highlights the inherent limitations of standard user-space Linux for microsecond-scale deterministic control loops.

## Finding 3: Completion Path Dominates
Comparing the `Interrupt` and `Polling` modes reveals the cost of the kernel completion path.
- **Interrupt p50**: ~26µs (including ~6µs overhead)
- **Polling p50**: ~21µs (including ~1µs overhead)
The cost of context switching and softirq processing for interrupt delivery adds measurable latency and jitter compared to a tight polling loop.

## Finding 4: Memory Setup Matters
The `FixedBuf` benchmark demonstrates the impact of removing memory setup (like `pin_user_pages`) from the hot loop.
- **Baseline**: ~40µs (includes 10µs setup)
- **FixedBuf**: ~23µs (removes 10µs setup)
Pre-registering memory buffers is essential for ultra-low latency inference to avoid repeated kernel entries for memory management.
