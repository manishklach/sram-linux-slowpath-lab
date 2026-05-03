# Existing io_uring Fast Path Validation

## Setup
- **Environment**: WSL (Synthetic) / Native Linux (Baseline)
- **CPU Pinning**: Requests pinned to core 0.
- **Kernel Version**: 6.19.7 (Experimental with attribution tracepoints).
- **Workloads**: 
  - **NOP**: Raw `io_uring` overhead measurement.
  - **SRAM20**: Deterministic 20µs busy-wait compute model.

## Track A: Pure io_uring NOP
This track measures the raw ring overhead without any application-level compute.
- **Goal**: Establish the "floor" of `io_uring` latency on the current host.
- **Note**: WSL results in this mode are useful only as a harness sanity check; hypervisor-induced jitter is highly visible at the sub-microsecond scale.

| Mode | p50 (µs) | p99 (µs) | p99.9 (µs) | Status |
|------|----------|----------|------------|--------|
| Baseline | - | - | - | Pending |
| SQPOLL | - | - | - | Pending |
| SQPOLL + Fixed Buffers | - | - | - | Pending |
| SQPOLL + Buffers + Files | - | - | - | Pending |

## Track B: SRAM-style deterministic inference
This track includes a deterministic ~20µs busy-wait per request, simulating a zero-variance SRAM-based hardware accelerator.
- **Goal**: Better matches the project's core thesis. 
- **Goal**: Used to evaluate whether host overhead (completion delivery and scheduling) remains visible relative to predictable compute.

| Mode | p50 (µs) | p99 (µs) | p99.9 (µs) | Status |
|------|----------|----------|------------|--------|
| Baseline | - | - | - | Pending |
| SQPOLL | - | - | - | Pending |
| SQPOLL + Fixed Buffers | - | - | - | Pending |
| SQPOLL + Buffers + Files | - | - | - | Pending |

## Interpretation of Gaps

- **Does SQPOLL eliminate submission overhead?**: (To be answered after validation)
- **Do registered buffers reduce memory overhead?**: (To be answered after validation)
- **Does completion → wakeup still dominate?**: **THIS IS THE KEY QUESTION.**
- **Interpretation Principle**: In WSL NOP-scale runs, SQPOLL may add overhead for this minimal workload due to hypervisor context switching; native Linux attribution is required before drawing final conclusions.

## Conclusion

Even after applying all existing `io_uring` fast paths, we evaluate whether the completion and scheduling pipeline remains a significant contributor to tail latency in microsecond-scale inference workloads. This empirical baseline will determine the path forward for Phase 7/8.
