# Project Roadmap: SRAM Inference Fastpath

This roadmap tracks the evolution from latency attribution to kernel-level optimization.

## Phase 1: Deterministic AI inference model
- **Goal**: Create a zero-variance simulated device.
- **Status**: Complete.

## Phase 2: Linux slow-path attribution
- **Goal**: Identify the exact cost of GUP, DMA, and context switches.
- **Status**: Complete.

## Phase 3: Fastpath runtime prototype
- **Goal**: Model the ideal latency of a fully optimized system.
- **Status**: Complete.

## Phase 4: Kernel tracepoint patches
- **Goal**: Export RFC-grade instrumentation for submit/issue/complete.
- **Status**: Complete.

## Phase 5: Low-latency io_uring mode
- **Goal**: Implement `IORING_SETUP_LOW_LATENCY` UAPI.
- **Status**: Complete.

## Phase 6: Registered-buffer and wakeup attribution
- **Goal**: Instrument fixed-buffer paths and task-wakeup delays.
- **Status**: Complete.

## Phase 7: Existing io_uring fastpath gap analysis
- **Goal**: Analyze the residual latency when SQPOLL, IOPOLL, and Registered Buffers are used.
- **Status**: Complete.

## Phase 8: Native Linux validation
- **Goal**: Benchmark on bare-metal hardware to eliminate hypervisor jitter.
- **Status**: In-progress.

## Phase 9: Submission path optimization analysis
- **Goal**: Focus research on reducing `submit→issue` latency based on native evidence.
- **Status**: Current.

## Phase 10: Kernel patch only after native evidence
- **Goal**: Propose submission-side optimizations (e.g., SQPOLL affinity, allocation caching) only if data justifies it.
- **Status**: Planned.
