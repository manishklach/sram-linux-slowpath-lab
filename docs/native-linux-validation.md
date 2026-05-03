# Native Linux Validation Guide

## Why Native Validation is Required

While WSL2 provides a convenient development environment, it introduces several artifacts 
that make microsecond-scale latency attribution unreliable:
- **Hypervisor Scheduling**: The WSL2 VM is subject to the Windows host scheduler. A kernel 
  thread (like `io_uring` SQPOLL) may be de-scheduled by the host, leading to artificial 
  tail latency.
- **IPI and Interrupt Latency**: Signal delivery between the virtualized Linux kernel and 
  the host hardware is not zero-cost.
- **Clock Source Jitter**: While `CLOCK_MONOTONIC_RAW` is used, the virtualization layer 
  can still introduce micro-jitter.

To definitively prove the project's thesis, we must measure on **bare-metal Linux**.

## Pre-flight Checklist

- [ ] **Bare-metal Linux**: No virtual machine or hypervisor (KVM/Hyper-V).
- [ ] **CPU Isolation**: Target core is isolated via `isolcpus` or at least idle.
- [ ] **Fixed Frequency**: CPU frequency scaling (Intel P-States/AMD PBO) is disabled.
- [ ] **Tools Ready**: `bpftrace` and `gcc` are installed and functional.
- [ ] **No Background Load**: No heavy compilation or I/O occurring during measurement.

## Environment Requirements

- **Kernel**: Linux >= 5.15 (for reliable `io_uring` tracepoints).
- **Access**: Root/Sudo (required for `bpftrace` and `taskset`).
- **Isolation**: Support for `taskset` or `isolcpus`.

## Step-by-Step Validation Procedure

### Step 1: Isolate the CPU
```bash
# Pin to core 0 (or any isolated core)
taskset -c 0 ./scripts/run_existing_fastpath_validation.sh 10000
```

### Step 2: Run Latency Attribution (BPF)
```bash
sudo bpftrace trace/bpf/io_uring_latency.bt > trace_attribution.log
```

### Step 3: Targeted Benchmark Run
```bash
./benchmarks/io_uring_real --workload=sram20 --mode=A --iters=10000
```

## Attribution Goals

We want to quantify the time spent in each segment:
1.  **submit → issue**: Submission side overhead (system call + request prep).
2.  **issue → complete**: The "Hardware" time (simulated by 20µs busy-wait).
3.  **complete → wakeup**: Kernel completion posting and waitqueue signal.
4.  **wakeup → sched-in**: From kernel wakeup to the user process running.

## Expected Signals

- **If a gap exists**: `complete → wakeup` or `wakeup → sched-in` will be large relative 
  to the 20µs compute time.
- **If not**: Existing `io_uring` APIs are sufficient for deterministic workloads.

## What Would Justify CQ Polling?

We will only proceed with experimental patches if:
1.  **`complete → wakeup` dominates the p99 tail** (> 5-10µs residual).
2.  **`wakeup → sched-in` is relatively small**, indicating the process is ready but 
    the kernel is slow to notify.
3.  **Userspace is actively polling** but still incurs high kernel overhead.
