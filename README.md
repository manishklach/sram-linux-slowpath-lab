# SRAM Linux Slowpath Lab

A WSL-compatible lab demonstrating how deterministic SRAM-style inference exposes Linux/host overhead as the dominant latency source.

## Overview
This repository provides synthetic benchmarks that model ultra-low latency, deterministic SRAM device execution (e.g., ~20μs inference times). At these microsecond scales, the overhead of the Linux kernel slowpath—memory setup, context switches, and interrupt handling—often dwarfs the actual device execution time.

**Note:** This version is built to run entirely inside Ubuntu WSL. It models the latency behavior deterministically to prove the method and the latency model. It does not measure real kernel IRQ behavior, but simulates it to show its relative impact.

## Quick Start

Ensure you have `gcc` and `python3` installed.

Build the benchmarks:
```bash
./scripts/build.sh
```

Run all benchmarks and summarize the results:
```bash
./scripts/run_all.sh
```

### Expected Output Example
```
=== Baseline ===
Metric          | Min      | p50      | p95      | p99      | p999     | Max      | Avg     
-------------------------------------------------------------------------------------
submit_ns       | 4900     | 5020     | 5100     | 5200     | 5500     | 6000     | 5050.00 
device_ns       | 20000    | 20050    | 20100    | 20200    | 20500    | 21000    | 20060.00
completion_ns   | 2900     | 3000     | 3100     | 3200     | 3500     | 4000     | 3020.00 
total_ns        | 27800    | 28070    | 28300    | 28600    | 29500    | 31000    | 28130.00
```

## What this repo proves

- **Deterministic compute does NOT eliminate latency variance**: Even with a 100% deterministic device simulation, OS-level scheduling introduces significant tail latency.
- **Linux submission + completion path dominates**: At microsecond scales (20µs device time), the time spent in the host kernel and userspace prep often exceeds 50% of the total request time.
- **Removing GUP and interrupt paths reduces latency significantly**: Pre-registering memory (Fixed Buffers) and using Polling are critical for deterministic performance.

## Native Linux Tracing Preview

This repository is designed in two phases:

1. **WSL Mode (Synthetic Model)**: Models latency using high-resolution timers and deterministic busy-waits. Proves the logic that software overhead dominates at microsecond scales.
2. **Native Linux Mode (Kernel-Path Attribution)**: Uses `bpftrace` on real hardware to measure the true costs of context switches, IRQs, and scheduler wakeups.

The tools for Native Linux are available in the `trace/` directory.

## Key Insight

- **Median latency ≈ device + small overhead**: At p50, the hardware and software overheads are comparable.
- **Tail latency ≫ device**: At p99 and beyond, latency is driven by:
  - **Scheduler**: Thread preemption and wakeup delays.
  - **Completion path**: Context switches for interrupt/softirq handling.
  - **OS noise**: Background tasks and kernel maintenance.

## What this means

- **Optimizing compute alone is insufficient**: Transitioning to SRAM or faster accelerators only improves the floor (median), but not the ceiling (tail).
- **Control plane must be optimized**: Real-time kernels, core isolation, and userspace drivers (like io_uring with polling) are required to tame the tail.

## Limitations (IMPORTANT)

- **WSL-based**: This lab runs in WSL2. While excellent for synthetic modeling, it does not represent native hardware performance.
- **No real IRQ/softirq tracing**: The kernel paths are simulated via busy-waits and variable delays.
- **Extreme tail latency**: p999+ results are heavily influenced by the host OS (Windows) and the hypervisor.

## Next Phase (Native Linux)

To validate these findings on real hardware, future work involves using `bpftrace` and `ftrace` on a native Linux installation to trace:
- `sys_enter_io_uring_enter`
- `pin_user_pages`
- `dma_map_sg`
- `irq_handler_entry/exit`
- `softirq_entry/exit`
- `sched_wakeup`
- `sched_switch`
