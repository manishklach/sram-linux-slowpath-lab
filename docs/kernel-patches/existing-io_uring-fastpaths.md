# Existing io_uring Fast Paths and Remaining Gaps

## Why this document exists
Before proposing new kernel behavior or attribution mechanisms, it is essential to compare the current prototype against established `io_uring` fast paths. This project does not aim to replace existing mechanisms but rather to identify and measure the residual latency that remains once they are fully utilized.

## IORING_SETUP_SQPOLL
- **What it does**: Polls the submission queue (SQ) from a dedicated kernel thread.
- **Benefits**: Reduces submission-side syscall overhead by allowing userspace to post work without entering the kernel.
- **Remaining Gap**: SQPOLL primarily addresses the *submission* path. It does not eliminate the latency involved in completion delivery, task wakeups, or the scheduling delay between a completion being posted and the user task actually seeing it in userspace.

## IORING_SETUP_IOPOLL
- **What it does**: Enables the kernel to poll for I/O completions on supported, polling-capable file or device paths (typically block devices with O_DIRECT).
- **Benefits**: Significant reduction in interrupt-driven latency for high-speed storage.
- **Remaining Gap**: `IOPOLL` is not a universal attribution mechanism for all accelerator-style inference loops. It is often restricted to specific file/device types and does not provide visibility into the wakeup-to-sched-in latency for non-IOPOLL paths or general-purpose compute completion signals.

## IORING_REGISTER_BUFFERS
- **What it does**: Allows userspace to pre-map and pin memory regions into the kernel.
- **Benefits**: Reduces repeated memory setup, page pinning (GUP), and translation overhead during every I/O operation.
- **Remaining Gap**: Registered buffers effectively address submission and memory-mapping overhead. However, they do not address the completion delivery pipeline, wakeup attribution, or the noise introduced by the host scheduler.

## Fixed files
- **What it does**: Allows applications to register a set of file descriptors with the ring.
- **Benefits**: Reduces the overhead of per-request file descriptor lookups and reference counting.
- **Remaining Gap**: While valuable for throughput and submission efficiency, fixed files are orthogonal to the microsecond-scale completion and wakeup latency targeted by this project.

## What remains after using existing fast paths
Once SQPOLL, IOPOLL, and Registered Buffers are applied, several microsecond-scale gaps persist:
- **Completion posted → CQ visible**: The delay for the hardware signal to result in a CQE visible to userspace.
- **CQ visible → Userspace consumes**: The reaction time of the user-space application.
- **Completion posted → Wakeup requested**: The internal kernel delay before triggering a task wakeup.
- **Wakeup requested → Task scheduled**: The time the task spends on the runqueue before being context-switched in.
- **Tail latency**: The impact of host scheduling noise on deterministic execution.

## Why attribution comes before new policy
Before proposing new kernel policies (such as bounded CQ polling or wakeup avoidance), this project prioritizes **attribution**. We need to measure the precise cost of:
- Submit → Issue
- Issue → Complete
- Complete → Wakeup
- Wakeup → Sched-in
- CQ consumed vs. Wakeup fallback

## Conclusion
Existing `io_uring` fast paths are valuable and should be used first. This project builds on `SQPOLL`, `IOPOLL`, and registered buffers—it does not replace them. It focuses on the residual completion, wakeup, and scheduler latency that remains after those mechanisms are applied, providing the empirical data necessary to justify further kernel-level optimizations.
