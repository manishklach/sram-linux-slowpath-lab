# Kernel Patch Proposals for SRAM-Based Inference Slow Paths

## Context

SRAM-style deterministic inference reduces accelerator-side variance, making the host-side submission, completion, and scheduling paths the dominant sources of latency and jitter. This repository is building the experimental evidence required to propose targeted kernel changes to address these slow paths.

## Proposal 1: Persistent DMA-Mapped Inference Buffers

**Problem**: The per-request overhead of `pin_user_pages` (GUP) and `dma_map_sg` adds significant latency and jitter, especially for small, high-frequency inference requests.

**Idea**: Allow inference runtimes to register long-lived, DMA-mapped regions. This avoids repeated page pinning and mapping in the hot path.

- **Kernel Areas**: `io_uring` registered buffers, DMA mapping APIs, device driver integration.
- **Risks**: Long-term pinned memory pressure, memory reclaim interference, security/isolation between users.

## Proposal 2: Polling Completion Mode for Latency-Critical Inference

**Problem**: The traditional `IRQ -> softirq -> wakeup -> scheduler` path introduces non-deterministic delays that scale poorly at the tail.

**Idea**: Allow dedicated inference service threads to poll completion queues directly, bypassing the interrupt delivery path entirely for latency-critical workloads.

- **Kernel Areas**: `io_uring` polling, driver completion queues, IRQ affinity.
- **Risks**: Increased CPU utilization, fairness impact on other tasks, power efficiency.

## Proposal 3: Latency-Sensitive Scheduler Hint for Inference Threads

**Problem**: Even after a wakeup is signaled, a thread may experience significant delay (`wakeup_to_sched_ns`) before being scheduled in by the kernel.

**Idea**: Introduce an explicit latency-sensitive hint for inference completion threads to prioritize their wakeup-to-run behavior without requiring full real-time (SCHED_FIFO) privileges.

- **Kernel Areas**: Scheduler wakeup path, cgroup integration, `sched_ext`, `latency_nice`.
- **Risks**: Task starvation, abuse by generic workloads, overall fairness impact.

## Proposal 4: Standardized Tracepoints for Accelerator Submission/Completion

**Problem**: Current generic tracepoints do not expose device-specific attribution, making it difficult to debug end-to-end latency without custom driver-specific tracing.

**Idea**: Add standardized kernel tracepoints around the accelerator lifecycle: submission, DMA mapping, doorbell signaling, hardware completion, and userspace notification.

- **Kernel Areas**: `drivers/accelerator`, `trace/events`, `io_uring`.
- **Risks**: Tracepoint ABI stability, handling vendor-specific hardware nuances.
