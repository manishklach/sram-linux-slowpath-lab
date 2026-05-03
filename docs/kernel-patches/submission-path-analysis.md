# Submission Path Latency Analysis

## Why this document exists
The project originally considered completion-side wakeup avoidance (e.g., `IORING_SETUP_CQ_POLL`). However, current attribution measurements show that completion-to-userspace residual latency is sub-microsecond in our deterministic benchmarks. The next justified research direction is a deep-dive into the submission-side path, which shows a much stronger latency signal.

## Current finding
- **Deterministic SRAM-style compute**: Fixed at ~20µs.
- **submit→issue p99**: Significant signal (~7µs in synthetic environments).
- **Residual completion path**: Sub-microsecond (~30–90ns) in current synchronous synthetic setups.
- **CQ polling justification**: Currently **NOT justified** by the available data.

## What submit→issue includes
The path from a userspace submission to the kernel issuing the request involves:
- **Userspace writing SQEs**: Posting entries to the ring and updating the tail.
- **Memory ordering**: Ring visibility and atomic barrier overhead.
- **io_uring_enter syscall**: The context switch cost if `SQPOLL` is not used.
- **SQ ring processing**: Kernel-side consumption and validation of SQEs.
- **Request preparation**: Mapping userspace data into internal kernel structures.
- **Request allocation / caching**: Allocation of `struct io_kiocb` and related state.
- **Issue dispatch**: Handing the request off to the device driver or internal worker.

## Why submission matters for SRAM-style inference
When hardware execution is tens of microseconds and highly predictable (deterministic), even a few microseconds of submission overhead becomes visible at p99. This overhead represents a fixed "tax" on every inference request, regardless of model size.

## Existing mechanisms

### SQPOLL
- **What it does**: Uses a kernel thread to poll the SQ ring.
- **Benefits**: Reduces submission-side syscall overhead.
- **Challenges**: Sensitive to thread scheduling and CPU placement; can lead to cacheline bouncing.

### Registered buffers
- **What it does**: Pre-maps memory regions into the kernel.
- **Benefits**: Reduces memory setup and page pinning overhead per request.

### Fixed files
- **What it does**: Registers file descriptors with the ring.
- **Benefits**: Reduces fd lookup and reference counting overhead.

## Hypotheses to test next
1. **H1**: `SQPOLL` only helps significantly when the polling thread is scheduled predictably (low jitter).
2. **H2**: CPU placement (NUMA distance, sibling threads) between the userspace submitter and the `SQPOLL` thread affects p99 submission latency.
3. **H3**: Batching multiple requests reduces the per-request `submit→issue` overhead by amortizing syscall and allocation costs.
4. **H4**: Fixed buffers and files reduce setup overhead but do not eliminate the core ring-processing latency.

## Experiments
- **Experiment 1**: Baseline vs `SQPOLL` on native Linux to isolate syscall impact.
- **Experiment 2**: `SQPOLL` with submitter pinned to the **same CPU** vs. a **different CPU** core.
- **Experiment 3**: Batch size sweep (1, 2, 4, 8, 16) to measure amortization.
- **Experiment 4**: Registered buffers + fixed files vs. baseline on native hardware.
- **Experiment 5**: Native `bpftrace` attribution of the `submit→issue` tail latency.

## Future kernel directions
Possible directions (subject to measurement results):
- **Better SQPOLL affinity controls**: Providing userspace more influence over kernel thread placement.
- **Submission-side tracepoints**: Exporting more fine-grained metrics for `io_kiocb` lifecycle.
- **Request allocation/caching analysis**: Optimizing the slab cache behavior for high-frequency low-latency loops.
- **Cache-local ring processing**: Designing ring structures that minimize cross-core cache invalidation.
- **Batching-aware latency mode**: A mode that optimizes for per-batch rather than per-request delivery.

## Why not CQ polling yet
CQ polling targets completion and wakeup latency. Current attribution data does not show that path as dominant in deterministic workloads. As a result, CQ polling remains a future experiment only if native Linux attribution eventually justifies it.
