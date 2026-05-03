# Project Roadmap: SRAM Inference Fastpath

This roadmap tracks the evolution from latency attribution to kernel-level optimization.

## Phase 1: Deterministic AI inference model
- **Goal**: Create a zero-variance simulated device.
- **Status**: Complete.
- **Artifact**: `benchmarks/io_uring_real.c` with deterministic busy-waits.

## Phase 2: Linux slow-path attribution
- **Goal**: Identify the exact cost of GUP, DMA, and context switches.
- **Status**: Complete.
- **Artifact**: `trace/bpf/io_uring_latency.bt`.

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

## Phase 7: Existing io_uring fastpath gap analysis and wakeup attribution
- **Goal**: Analyze the residual latency when SQPOLL, IOPOLL, and Registered Buffers are used.
- **Status**: Planned.

## Phase 8: Optional bounded wakeup-avoidance experiment, only if measurements justify it
- **Goal**: Prototype experimental wakeup avoidance policies if attribution data confirms significant residual overhead.
- **Status**: Planned.

## Phase 9: Native Linux validation
- **Goal**: Benchmark on bare-metal hardware to eliminate hypervisor jitter.
- **Status**: In-progress.

## Phase 10: Vendor/hardware integration
- **Goal**: Partner with hardware vendors to validate against real SRAM accelerators.
- **Status**: Planned.
