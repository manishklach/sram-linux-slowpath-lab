# Experimental Kernel Improvement: Low-Latency Mode

Based on our findings that Linux completion path overhead dominates deterministic inference latency, we have implemented an experimental kernel patch: `IORING_SETUP_LOW_LATENCY`.

## Mapping to Fastpath Prototype
In our `runtime/fastpath.c` prototype, we demonstrated that:
1. Pinned CPUs reduce scheduler variance.
2. Polling completions bypasses interrupt/wakeup delays.

The `IORING_SETUP_LOW_LATENCY` kernel flag brings these optimizations into the core `io_uring` subsystem.

## Implementation Details

### 1. Busy-Wait in `io_cqring_wait`
When an application waits for completions via `io_uring_enter`, the kernel now performs a bounded busy-loop before sleeping. This captures "near-immediate" completions from deterministic hardware without incurring a context switch.

### 2. Aggressive Notification
The patch forces `TWA_SIGNAL` notification for all task-work associated with the ring. This ensures that completion visibility is not delayed by cooperative scheduling policies.

### 3. Integrated Instrumentation
The `io_uring_complete` tracepoint has been extended with a `low_latency` bit, allowing researchers to verify that the optimized path was indeed utilized for a specific request.

## Expected Results
With this mode enabled, we expect the `Total Latency` to closely track the `Device Latency`, effectively removing the "Slow Path" peaks (p99/p999) caused by scheduler wakeups and interrupt handling.

```text
Total Latency (Low-Latency Mode) ≈ Device Latency + Constant Minimal Overhead
```

## Status
This is a research prototype. It is not intended for upstream Linux submission in its current form due to the significant increase in CPU power consumption during polling.
