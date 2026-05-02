# Kernel Patch: Wakeup Attribution

## Problem: "Wakeup != Execution"
In asynchronous systems, a request is "completed" by the kernel, and a "wakeup" is sent to the userspace process. However, the process does not run immediately. It must wait for the scheduler to pick it up.

In high-load or jittery environments, this `wakeup -> sched_in` delay can be hundreds of microseconds, even if the device completed the work in 20µs.

## Solution
This patch adds an `io_uring_cq_wakeup` tracepoint specifically at the moment the kernel decides to signal the waiter.

By correlating this with `sched_wakeup` and `sched_switch`, we can accurately measure:
1. **Device Latency**: `issue -> complete`
2. **Wakeup Overhead**: `complete -> cq_wakeup`
3. **Scheduling Delay**: `cq_wakeup -> sched_in`

## Connection to Results
This allows us to prove that "Linux control plane dominates latency" not just in the submission path, but also in the scheduling path.
