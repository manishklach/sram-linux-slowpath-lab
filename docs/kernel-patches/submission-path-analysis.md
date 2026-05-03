# Submission Path Latency Analysis

## Overview
Based on recent high-fidelity measurements (see [Submission Results](../submission-results.md)), 
the primary bottleneck in deterministic AI inference workloads is the **Submission Path 
(`submit → issue`)**. Crucially, we have identified that **batching** is the most effective 
mechanism for reducing this overhead.

## Why batching works
Batching reduces the per-request submission latency by amortizing several fixed-cost 
operations:
- **Syscall overhead (`io_uring_enter`)**: The context switch cost is paid once for N requests.
- **SQ ring doorbell cost**: Ring visibility and atomic updates are handled as a single unit.
- **Kernel/Userspace transitions**: Minimizes the expensive transitions between protection 
  domains.
- **Request setup overhead**: Kernel-side SQE processing and initial request validation are 
  streamlined for multiple entries.
- **Cacheline bouncing**: Fewer updates to head/tail pointers reduce cross-core cache 
  invalidation cycles.

## Visual Comparison (Amortized Cost)

**Batch=1**: High tax per request
```text
[ io_uring_enter ] [ request ]
|------- FIXED --------|
```

**Batch=16**: Low tax per request
```text
[ io_uring_enter ] [ req ][ req ][ req ][ req ][ req ][ req ] ...
|------- FIXED --------|
```

## Submission Path Cost Model
The cost of submitting a batch of size N can be modeled as:

**Cost(N) = Fixed + (N * Per-request)**

Where:
- **Fixed**: System call transition, ring processing setup, and initial task notification.
- **Per-request**: `io_kiocb` allocation, SQE copying, and minimal bookkeeping.

By increasing N, the **Fixed/N** component asymptotically approaches zero, leaving only 
the highly optimized per-request path.

## Why p99 does not scale with batch size
Observation: The p99 for a batch of 16 (~6.4µs) is only slightly higher than for a single 
request (~5.4µs).
- **Unit Processing**: The kernel processes the entire batch as one execution unit once 
  inside the syscall.
- **Amortized Scheduling**: The scheduling overhead (wait times, context switches) is paid 
  per batch, not per individual request.
- **Tail Events**: A jitter event (e.g., an interrupt) affects the entire batch once, 
  rather than hitting each request independently.

## Implication for AI inference
- **Batching is more powerful than kernel tweaks**: Even an optimized kernel fast path 
  cannot beat the mathematical efficiency of amortization.
- **Complementary Fast Paths**: Future kernel fast paths should focus on complementing 
  batching (e.g., reducing the per-request allocation cost) rather than trying to replace it.
- **System Design Matters**: High-performance inference design must prioritize batch-aware 
  submission loops to reach the nanosecond latency regime.

## Future Experiments
- **Batch Size vs Throughput vs Latency**: Identifying the "sweet spot" where batching gains 
  begin to hit diminishing returns due to cache effects.
- **Mixed Workloads**: Measuring batching effectiveness when NOPs are mixed with real I/O.
- **Batching + SQPOLL**: Validating on native Linux whether SQPOLL + Batching can eliminate 
  the fixed cost entirely without introducing tail jitter.

## Conclusion
The data proves that the submission plane is currently a major contributor to host overhead. 
However, batching provides a massive (~7x) reduction in per-request tax, shifting the 
optimization priority toward maximizing submission density.
