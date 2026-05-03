# Existing io_uring Fast Path Validation

## Setup
- **Environment**: WSL (Synthetic) / Native Linux (Baseline)
- **CPU Pinning**: Requests pinned to core 0.
- **Kernel Version**: 6.19.7 (Experimental with attribution tracepoints).
- **Workload**: `IORING_OP_NOP` with deterministic hardware simulation.

## Results Summary

| Mode | p50 (µs) | p99 (µs) | p99.9 (µs) | Status |
|------|----------|----------|------------|--------|
| Baseline | - | - | - | Pending |
| SQPOLL | - | - | - | Pending |
| SQPOLL + Fixed Buffers | - | - | - | Pending |
| SQPOLL + Buffers + Files | - | - | - | Pending |

## Latency Attribution Breakdown

Once benchmarks are executed, this section will provide the microsecond breakdown of the following segments:

### 1. Submit → Issue (Control Plane Setup)
- **Expectation**: `SQPOLL` should eliminate the syscall overhead.
- **Goal**: Measure the residual cost of `io_uring` internal request allocation and setup.

### 2. Issue → Complete (Hardware/Compute)
- **Expectation**: Deterministic in this simulated environment.
- **Goal**: Verify that compute jitter is not the source of tail latency.

### 3. Complete → Wakeup (Kernel Pipeline)
- **Expectation**: Potential bottleneck due to `io_cqring_wake` and waitqueue management.
- **Goal**: Identify if the kernel spends significant time before issuing the wakeup signal.

### 4. Wakeup → Sched-in (Host Scheduling)
- **Expectation**: Major contributor to tail latency (p99+).
- **Goal**: Quantify the cost of the task being ready but not yet running.

## Interpretation of Gaps

- **Does SQPOLL eliminate submission overhead?**: (To be answered after validation)
- **Do registered buffers reduce memory overhead?**: (To be answered after validation)
- **Does completion → wakeup still dominate?**: **THIS IS THE KEY QUESTION.** If the delay between the kernel completing a request and the task becoming active remains >10µs, it justifies the exploration of bounded CQ polling or wakeup avoidance.

## Conclusion

Even after applying all existing `io_uring` fast paths, we hypothesize that the completion and scheduling pipeline remains a significant contributor to tail latency in microsecond-scale inference workloads. This empirical baseline will determine the path forward for Phase 7/8.
