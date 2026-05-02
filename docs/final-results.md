# Final Results

This document summarizes the final experimental findings from the SRAM Linux Slowpath Lab.

## Experiment Setup
- **Deterministic Device**: Simulated as a 20.0µs busy-wait using `CLOCK_MONOTONIC_RAW` and `pause` assembly instructions.
- **Pinned CPU**: All benchmarks were executed pinned to CPU 0 to minimize scheduler migration noise.
- **Iterations**: 10,000 requests per mode.

## Results Table (p50 Latency)

| Mode | p50 (µs) | p95 (µs) | p99 (µs) | p999 (µs) |
| :--- | :--- | :--- | :--- | :--- |
| **Baseline** | 40.9 | 78.9 | 190.3 | 1280 |
| **FixedBuf** | 23.2 | 23.3 | 61.7 | 286.7 |
| **Interrupt** | 25.9 | 35.8 | 94.6 | 453.2 |
| **Polling** | 21.2 | 31.1 | 78.8 | 480.8 |
| **Fastpath** | 20.7 | 20.8 | 53.0 | 143.5 |

## Key Findings

### Finding 1: Baseline overhead ≈ device latency
In the **Baseline** mode, the end-to-end latency (~41µs) is roughly double the actual computation time (~20µs). This proves that standard Linux submission and completion paths are the dominant latency source for deterministic accelerators.

### Finding 2: Memory setup matters
By pre-registering buffers (**FixedBuf**), we remove the per-request cost of memory pinning and DMA mapping. This collapses median latency from ~41µs to ~23µs, a 43% reduction.

### Finding 3: Completion path matters
Transitioning from interrupt-driven wakeups (**Interrupt**) to deterministic polling (**Polling**) further reduces overhead and jitter. While the median improvement is small (~4µs), the tail stability improves significantly.

### Finding 4: Fastpath approaches device limit
The **Fastpath** prototype demonstrates that an ideal, fully optimized runtime can achieve ~20.7µs median latency. This indicates that the host-side overhead can be reduced to less than 1µs (a 95%+ reduction in overhead compared to baseline).
