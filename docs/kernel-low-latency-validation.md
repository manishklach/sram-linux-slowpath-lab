# Kernel Low-Latency Validation

This document outlines the methodology and expected results for validating the `IORING_SETUP_LOW_LATENCY` patch.

## Expected Result

- **Lower Completion Latency**: By busy-polling in `io_cqring_wait`, we expect to capture completions that occur shortly after the `io_uring_enter` call without incurring a context switch.
- **Lower p99 Variance**: Avoiding the scheduler wakeup path for near-immediate completions should significantly reduce the tail latency peaks seen in standard interrupt-driven modes.
- **Higher CPU Usage**: The polling loop in the kernel will consume 100% of the calling CPU for the duration of the busy-wait (currently bounded to ~1 jiffy or a fixed iteration count).

## Failure Modes

- **No Improvement**: This indicates that completions are occurring outside the busy-wait window, or that the "device" latency is so high that the polling overhead is negligible.
- **Worse Latency**: If the polling loop overhead or the forced `TWA_SIGNAL` IPIs cause more delay than the scheduler wakeup, the patch is counter-productive.
- **Unstable Results (WSL)**: WSL's hypervisor-managed scheduling can introduce artificial variance that masks the kernel's internal behavior. **Native Linux is required for high-fidelity validation.**

## Methodology

1. Apply the kernel patch series.
2. Rebuild and boot the patched kernel.
3. Run `scripts/run_kernel_low_latency_validation.sh`.
4. Analyze the delta between `io_uring_normal.jsonl` and `io_uring_low_latency.jsonl`.

---

**Note**: This is an experimental research prototype. It has not been validated on real SRAM hardware and is intended for local measurement of control plane overhead only.
