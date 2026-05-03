# SRAM Inference Kernel Fastpath

### TL;DR
- **The Problem**: In deterministic AI inference (~20µs), Linux host overhead (interrupts, wakeups, GUP) can match or exceed compute time.
- **The Solution**: This repo prototypes the missing Linux kernel fast paths required to remove that gap.
- **Key Takeaway**: Once AI inference becomes deterministic, the OS becomes the bottleneck.

## Core Claim
> **SRAM-based inference shifts the bottleneck from compute to the Linux control plane. This repo prototypes the missing kernel fast paths required to close that gap.**

---

Experimental Linux fast-path patches for SRAM-based AI inference servers.

When device execution is deterministic and low-variance (often in the tens of microseconds), the Linux host overhead—previously hidden by compute jitter—becomes the primary bottleneck. This project implements the missing layer for next-generation AI infrastructure: dedicated fast paths for submission, buffer registration, completion delivery, and attribution.

## Why SRAM-Based AI Inference Servers Need Linux Fast Paths

- **Predictable Device Execution**: Device execution can be tens of microseconds with near-zero variance.
- **Comparable Host Overhead**: Standard Linux control plane overhead (interrupts, softirqs, context switches) is often comparable to device execution.
- **General-Purpose Bottlenecks**: Existing Linux I/O and scheduling paths are designed for general-purpose workloads, not microsecond-level determinism.
- **Need for Fast Paths**: High-performance accelerators require bounded, measurable, and opt-in kernel fast paths that bypass legacy slow-path logic.

**Without these fast paths, hardware improvements are not visible at the application level.**

## Missing Kernel Pieces This Repo Prototypes

This repo prototypes the kernel-level fast paths required for low-latency AI inference:

- **io_uring Latency Tracepoints**: High-resolution instrumentation for request lifecycle tracking.
- **Registered-Buffer Attribution**: Measurement of the performance gap between pinned and unpinned memory paths.
- **Low-Latency Completion Mode**: An experimental `IORING_SETUP_LOW_LATENCY` flag to prioritize determinism.
- **Wakeup Attribution Tracepoints**: Capturing the delay between kernel completion and userspace task execution.
- **Bounded CQ Polling**: Bypassed interrupt paths using bounded, non-destructive busy-wait loops.
- **Fastpath Runtime Model**: A reference model for microsecond-scale inference request processing.
- **Validation Tooling**: BPF and trace-based tools for isolating kernel vs. device latency.

## Vendor-Relevant Questions

- How much of end-to-end request latency is spent in the device vs. the Linux host?
- Is completion latency dominated by hardware IRQs, softirqs, task wakeups, or scheduler delays?
- Do registered (fixed) buffers significantly reduce the per-request submission overhead?
- Does bounded polling effectively collapse p99 tail latency compared to interrupt-driven completion?
- Which specific kernel path must be optimized before hardware compute gains are visible to the application?

**These determine whether accelerator improvements translate into real-world latency gains.**

## Relationship to Existing io_uring Fast Paths

This project does not ignore existing `io_uring` mechanisms such as `SQPOLL`, `IOPOLL`, registered buffers, and fixed files. It builds on them. The open research question is the residual completion, wakeup, and scheduler latency that remains after those mechanisms are used.

For more details, see:
- [Existing io_uring Fast Paths and Remaining Gaps](docs/kernel-patches/existing-io_uring-fastpaths.md)
- [Maintainer FAQ](docs/maintainer-faq.md)

## What This Is / Is Not

**This is:**
- Experimental Linux kernel fast-path research and prototyping.
- Reproducible latency modeling for deterministic AI workloads.
- A collection of kernel patch prototypes for subsystem maintainer review.
- Validation tooling for hardware-software co-design.

**This is not:**
- Vendor-specific hardware benchmarking.
- An upstream-ready kernel patch set.
- A production-grade driver or firmware.
- A claim of performance on specific real-world SRAM hardware.

## Project Roadmap

1. **Phase 1: Deterministic AI inference model** (Complete)
2. **Phase 2: Linux slow-path attribution** (Complete)
3. **Phase 3: Fastpath runtime prototype** (Complete)
4. **Phase 4: Kernel tracepoint patches** (Complete)
5. **Phase 5: Low-latency io_uring mode** (Complete)
6. **Phase 6: Registered-buffer and wakeup attribution** (Complete)
7. **Phase 7: Bounded CQ Polling** (Planned)
8. **Phase 8: Native Linux validation** (In-progress)
9. **Phase 9: Vendor/hardware integration** (Planned)

## Limitations and Safety

- **WSL Jitter**: Synthetic experiments on WSL include hypervisor-induced latency; true p999 requires bare-metal validation.
- **Experimental Status**: These patches are for research purposes and have not been submitted to LKML.
- **Policy Isolation**: All changes are gated behind opt-in flags to ensure no impact on standard Linux workloads.

## Getting Started

```bash
# Build all benchmarks
./scripts/build.sh

# Run the full suite pinned to CPU 0
./scripts/run_all_pinned.sh
```

## Takeaway

**Once AI inference becomes deterministic, the OS becomes the bottleneck.**

---

### GitHub About Suggestion

**Description**:
Experimental Linux kernel fast-path patches for SRAM-based AI inference servers, targeting io_uring submission, registered buffers, CQ polling, wakeup attribution, and completion latency.

**Topics**:
linux-kernel, io-uring, ai-inference, sram, low-latency, kernel-fastpath, kernel-tracing, bpftrace, inference-systems, ai-infrastructure, systems-performance, performance-analysis, async-io, benchmarking, operating-systems
