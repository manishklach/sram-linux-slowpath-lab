# Submission Path Experiment Plan

## Goal
Quantify the impact of submission-side optimizations (affinity, batching, and fixed resources) 
on p99 latency in deterministic ~20µs workloads.

## Required Environment
- **Native Linux Preferred**: To eliminate hypervisor scheduling artifacts.
- **Tools**: `taskset`, `bpftrace`, `python3`.
- **Isolation**: Single-socket or isolated core recommended.

## Metrics
- **submit→issue p50/p99/p999**: Time from userspace posting to kernel issuing.
- **total latency p50/p99/p999**: End-to-end request time.
- **Variables**:
  - CPU placement (Same core vs. Different core).
  - Batch size (1 to 16).
  - SQPOLL Status (Enabled/Disabled).
  - Fixed resources (Buffers and Files).

## Execution Commands

### Build
```bash
./scripts/build.sh
```

### Baseline Run (Sync Syscall)
```bash
taskset -c 0 ./benchmarks/io_uring_real --workload=sram20 --mode=A
```

### SQPOLL Run (Async Kernel Thread)
```bash
# Measure impact of kernel-side polling
taskset -c 0 ./benchmarks/io_uring_real --workload=sram20 --mode=B
```

### SQPOLL + Registered Buffers
```bash
# Measure impact of memory setup avoidance
taskset -c 0 ./benchmarks/io_uring_real --workload=sram20 --mode=C
```

## Results Table Template

| Mode | CPU Placement | Batch Size | p50 (µs) | p99 (µs) | p999 (µs) | Notes |
|------|---------------|------------|----------|----------|-----------|-------|
| A    | Core 0        | 1          |          |          |           |       |
| B    | Core 0        | 1          |          |          |           |       |
| C    | Core 0        | 1          |          |          |           |       |
| E    | Core 0        | 1          |          |          |           |       |

## Analysis Goals
- Determine if `submit→issue` latency scales linearly with batch size.
- Isolate whether cross-core cache invalidation is a primary driver of `SQPOLL` jitter.
