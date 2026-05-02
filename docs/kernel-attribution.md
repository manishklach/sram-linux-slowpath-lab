# Kernel Attribution Analysis

By correlating userspace request markers with kernel tracepoints, we can now attribute latency to specific stages within the Linux kernel.

## What we can now measure

- **Syscall Entry Latency**: The time from the application calling `io_uring_enter` to the kernel execution starting.
- **IRQ Arrival Delay**: The delta between device completion (simulated) and the hard interrupt handler executing.
- **Softirq Processing Delay**: The time spent in deferred work before the scheduler is notified.
- **Wakeup Delay**: The time from the kernel signaling a wakeup to the thread actually being ready for the scheduler.
- **Scheduling Delay (`wakeup_to_sched_ns`)**: The time the thread spends in the `runnable` state before being switched in by the scheduler. This is the primary indicator of CPU contention and scheduler noise.

## What we cannot yet measure

- **Driver-specific DMA cost**: Without a real hardware driver, the exact cost of `dma_map_sg` is not visible.
- **Exact pin_user_pages timing**: This requires deeper instrumentation of the memory management subsystem.
- **Device-specific queues**: Latency occurring inside the device's own internal firmware or command queues is opaque to host-side tracing.

## Key Metric: `wakeup_to_sched_ns`

This metric represents the "Scheduling Gap". In a perfectly deterministic system, this should be near zero. In standard Linux, especially under load or in a virtualized environment like WSL, this gap can grow to tens or hundreds of microseconds, becoming the dominant source of tail latency.
