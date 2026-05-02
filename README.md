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

## Native Linux Extensions

While WSL is excellent for synthetic modeling, deploying this on native Linux allows for real kernel-path proof. Future extensions of this lab will include `bpftrace`/`ftrace` tracing for:
- `sys_enter_io_uring_enter`
- `pin_user_pages`
- `dma_map_sg`
- `irq_handler_entry/exit`
- `softirq_entry/exit`
- `sched_wakeup`
- `sched_switch`
