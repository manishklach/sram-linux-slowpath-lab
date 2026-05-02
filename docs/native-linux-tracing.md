# Native Linux Tracing

While the current WSL-based synthetic modeling is excellent for proving out the latency model and demonstrating the impact of deterministic compute, certain microsecond-scale behaviors can only be measured on native Linux hardware.

## Why Native Linux?

- **Real IRQ/Softirq Timing**: WSL runs on a hypervisor, which introduces its own interrupt delivery overhead and jitter. Native Linux allows us to measure real hardware interrupt latency.
- **Scheduler Wakeup Attribution**: Tracing the exact path from device IRQ to thread wakeup requires a real Linux kernel scheduler without hypervisor intervention.
- **Syscall Overhead**: Real measurements of `io_uring_enter` and other syscall overheads.
- **Memory Pinning (GUP)**: Operations like `pin_user_pages` and DMA mapping can be truly instrumented only when real hardware drivers are involved.
- **Hardware DMA Mapping**: Tracing the actual IOMMU and DMA mapping costs (`dma_map_sg`).

## Using the Trace Tools

This repository provides a set of `bpftrace` scripts to prepare for native Linux experiments. These tools allow for deep kernel-path attribution beyond what userspace timing can provide.
