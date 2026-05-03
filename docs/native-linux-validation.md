# Native Linux Validation Guide

## Why Native Validation is Required

While WSL2 provides a convenient development environment, it introduces several artifacts that make microsecond-scale latency attribution unreliable:
- **Hypervisor Scheduling**: The WSL2 VM is subject to the Windows host scheduler. A kernel thread (like `io_uring` SQPOLL) may be de-scheduled by the host, leading to artificial tail latency.
- **IPI and Interrupt Latency**: Signal delivery between the virtualized Linux kernel and the host hardware is not zero-cost.
- **Clock Source Jitter**: While `CLOCK_MONOTONIC_RAW` is used, the virtualization layer can still introduce micro-jitter.

To definitively prove the project's thesis, we must measure on **bare-metal Linux**.

## Environment Requirements

- **Kernel**: Linux >= 5.15 (for reliable `io_uring` tracepoints).
- **Access**: Root/Sudo (required for `bpftrace` and `taskset`).
- **Tools**: 
  - `bpftrace`
  - `gcc`
  - `python3` (with `numpy` for analysis)
- **Isolation**: Support for `taskset` or `isolcpus`.

## Step-by-Step Validation Procedure

### Step 1: Isolate the CPU
To minimize interference from other system tasks, we run the benchmark and tracing on a specific core.
```bash
# Pin to core 0 (or any isolated core)
taskset -c 0 ./scripts/run_existing_fastpath_validation.sh 10000
```

### Step 2: Run Latency Attribution (BPF)
In a separate terminal, start the eBPF collector to capture the kernel-side events.
```bash
sudo bpftrace trace/bpf/io_uring_latency.bt > trace_attribution.log
```

### Step 3: Targeted Benchmark Run
Execute a focused run for the SRAM20 workload in Baseline mode.
```bash
./benchmarks/io_uring_real --workload=sram20 --mode=A --iters=10000
```

## Attribution Goals

We want to quantify the time spent in each segment of the request lifecycle:
1.  **submit → issue**: Submission side overhead (system call + request prep).
2.  **issue → complete**: The "Hardware" time (simulated by our 20µs busy-wait).
3.  **complete → wakeup**: Kernel-side completion posting and waitqueue signal.
4.  **wakeup → sched-in**: The time from the kernel issuing the wakeup to the user process actually running.

## Interpretation Guide

The data collected will determine the project's direction:

- **If `complete → wakeup` is large**: The standard `io_uring` completion pipeline is the bottleneck. This justifies exploring a more direct completion path.
- **If `wakeup → sched-in` is large**: The Linux scheduler is the primary source of jitter. This may suggest that userspace-side polling is more effective than any kernel-side wakeup optimization.

## What Would Justify CQ Polling?

We will only proceed with the experimental `IORING_SETUP_CQ_POLL` patch if the following conditions are met:
1.  **`complete → wakeup` dominates the p99 tail latency** (> 5-10µs residual).
2.  **`wakeup → sched-in` is relatively small**, indicating the process is ready but the kernel is slow to "notice" or "notify".
3.  **Userspace is actively waiting** (polling the CQ) but the kernel still incurs high overhead in its internal completion logic.

Without this empirical evidence on native hardware, no new kernel APIs will be proposed.
