# Submission Path Latency Analysis

## Overview
Based on recent high-fidelity measurements (see [Native Latency Breakdown](../native-latency-breakdown.md)), the primary bottleneck in deterministic AI inference workloads is the **Submission Path (`submit → issue`)**, rather than the completion delivery path. This document analyzes the components of this path and identifies optimization opportunities.

## Components of the submit → issue path
1.  **Userspace Submission**: Ringing the SQ doorbell (tail update + release barrier).
2.  **io_uring_enter syscall**: The cost of the transition from userspace to kernel space.
3.  **SQ Ring Processing**: Kernel-side consumption of SQ entries.
4.  **Request Allocation**: Creation and initialization of the `struct io_kiocb` internal state.
5.  **Issue Dispatch**: Handing the request off to the device-specific or internal processing logic.

## Optimization Opportunities

### Syscall Overhead
The `io_uring_enter` system call remains a significant fixed cost (~0.5–1µs). While standard `io_uring` uses this to signal work, in microsecond-scale inference, this cost is highly visible.
- **SQPOLL Benefit**: Moving to `IORING_SETUP_SQPOLL` can eliminate this syscall entirely, provided the kernel thread is active.

### SQPOLL Behavior & Scheduling
The SQPOLL kernel thread itself is a source of latency:
- **Scheduling Sensitivity**: If the SQPOLL thread is de-scheduled, the submission "hangs" until it is context-switched back in.
- **Cache Locality**: Accessing the SQ ring from a kernel thread running on a different CPU core can lead to cacheline bouncing and cross-core synchronization delays.

### Ring Access Patterns
- **Cacheline Bouncing**: Frequent updates to the SQ tail/head pointers can cause cache contention between the user process and the SQPOLL thread.
- **Memory Ordering**: Atomic barriers required for ring synchronization add micro-delays that aggregate over millions of operations.

### Request Allocation
- **Slab/Cache Effects**: Allocation of `io_kiocb` structures is heavily optimized in the kernel, but cold caches or slab fragmentation can still drive p99 tail latency.

## Why submission path dominates in deterministic workloads
In standard asynchronous I/O (e.g., storage), the device latency (milliseconds) dwarfs the submission overhead. However, when compute is fixed and extremely low (~20µs), the submission latency (~5–10µs at the tail) becomes a first-order contributor, accounting for 25% or more of the total request lifecycle.

## Proposed Experiments (Attribution only)

### Experiment 1: Tight Affinity
Pin both the userspace process and the SQPOLL kernel thread to the same physical core (sibling threads). This tests whether shared L1/L2 caches reduce submission-to-issue latency.

### Experiment 2: Sycall-Free Operation
Enforce a 100% SQPOLL-only submission path. Measure the latency delta when the user process never issues `io_uring_enter` (by using a large `sq_thread_idle` value).

### Experiment 3: Dynamic Batching
Measure whether batching 4-8 inference requests per submission reduces the per-request "tax" of the submission path.

## Potential Kernel Directions (Future)
- **Reduce io_uring_enter overhead**: Explore ways to minimize the instruction path within the entry-point.
- **SQPOLL Scheduling Stability**: Investigate real-time scheduling classes or dedicated task isolation for SQPOLL threads in low-latency regimes.
- **Cache-Local Submission Rings**: Explore hardware-aligned ring layouts that minimize cross-core cache invalidation.

## Conclusion
The data indicates that the path from userspace "posting" to the kernel "issuing" is the current bottleneck. Future research will focus on stabilizing and accelerating this submission plane to match the performance of deterministic SRAM-based accelerators.
