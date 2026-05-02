# Latency Attribution Report

This report analyzes where time is spent during a deterministic SRAM inference request and how that distribution shifts at the tail.

## Attribution at p50 (Median)

At the median, the latency distribution is relatively balanced between host preparation, device compute, and completion signaling.

- **Device Compute**: ~20.0µs (Deterministic)
- **Host Submission**: ~10.0µs
- **Host Completion**: ~10.0µs
- **Total**: ~40.0µs

**Insight**: At p50, device compute accounts for roughly **50%** of the total round-trip time. The host overhead is significant but predictable.

## Attribution at p99 (Tail)

As we move to the 99th percentile, the host-side overhead begins to dominate.

- **Device Compute**: ~20.0µs (Still stable)
- **Host Submission**: ~20.0µs+
- **Host Completion**: ~40.0µs+
- **Total**: ~80.0µs+

**Insight**: At p99, the host-side overhead (submission + completion) accounts for over **75%** of the latency. The "deterministic" compute is now the minority component.

## Attribution at p999 (Extreme Tail)

At the extreme tail, the host-side completion path explodes due to OS scheduling and hypervisor noise.

- **Device Compute**: ~20.0µs
- **Host Completion**: 100.0µs+
- **Total**: 150.0µs+

**Insight**: At p999, the host completion path (interrupt signaling, softirqs, scheduler wakeups) can be **5x to 10x longer** than the actual computation time.

## Conclusion

> "At median latency, compute dominates. At tail latency, host-side completion and scheduling dominate."

Removing compute jitter (SRAM) is only the first step. To achieve true microsecond-scale determinism, the Linux control plane (submission/completion) must be optimized or bypassed.
