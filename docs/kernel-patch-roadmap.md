# Kernel Patch Roadmap for SRAM-style AI Inference

This roadmap outlines the multi-phase strategy for improving Linux kernel latency determinism in AI inference workloads.

## Current State
- **Patch Set A (Instrumentation)**: `io_uring` submit/issue/complete tracepoints. [Implemented]
- **Patch Set B (Low-latency mode)**: `IORING_SETUP_LOW_LATENCY` flag + busy-poll logic. [Implemented]

## Upcoming Patch Sets

### Patch Set C: Registered-Buffer Fast Path
**Goal**: Enforce and verify the use of registered (fixed) buffers when low-latency mode is active.
- **Status**: Implemented.
- **Why**: Registered buffers avoid per-request `pin_user_pages` and DMA mapping, which we identified as a major latency "slowpath" (~20µs overhead).
- **Changes**: Added `fixed_buffers` field to tracepoints and added a kernel warning for non-fixed IO in low-latency rings.

### Patch Set D: Completion Polling Refinement
**Goal**: Optimize the bounded busy-poll loop in `io_cqring_wait`.
- **Status**: Planned.
- **Why**: Current prototype uses a jiffies-based bound. We need more fine-grained control (e.g., CPU relax cycles) to balance power vs latency.

### Patch Set E: Wakeup Attribution
**Goal**: Measure the delay between kernel completion posting and userspace task execution.
- **Status**: Implemented.
- **Why**: Even if a request completes instantly, the task might not run for many microseconds due to scheduling delays. We need to attribute this "Wakeup != Execution" delay.
- **Changes**: Added `io_uring_cq_wakeup` tracepoint.

### Patch Set F: Documentation and Validation
**Goal**: Formalize performance claims with reproducible benchmarks.
- **Status**: In-progress.

## Why C and E are next?
We have proven that kernel overhead exists. Now we need to:
1. **(C)** Ensure users are using the "fastest" possible memory path when they ask for "Low Latency".
2. **(E)** Attribute the remaining latency to the scheduler vs the device.
