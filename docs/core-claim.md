# Core Claim

The findings of this laboratory support the following three systems engineering claims:

1. **Deterministic compute does NOT eliminate latency variance**: Even if the accelerator (SRAM) is 100% deterministic, the OS scheduler and interrupt delivery path introduce significant tail latency.
2. **Linux submission + completion path dominates latency**: For microsecond-scale inference (20µs), the time spent in the host kernel and userspace overhead exceeds the actual computation time.
3. **Optimized control planes collapse latency**: Removing per-request memory setup (GUP), interrupt delivery, and scheduler interference reduces end-to-end latency to near-hardware limits (~20.7µs).

> **Conclusion**: Once compute is deterministic, the OS becomes the bottleneck.
