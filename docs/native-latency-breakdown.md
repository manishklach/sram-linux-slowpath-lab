# Native Latency Attribution Breakdown

## Attribution Summary (WSL Synthetic)

The following table breaks down the latency for the `sram20` baseline workload (N=10,000). 

> [!WARNING]
> Due to the lack of eBPF support in this specific environment, the `complete→wakeup` and `wakeup→sched` segments are combined into the residual "Completion Consumption" metric measured from userspace.

| Segment               | p50 (µs) | p99 (µs) | p99.9 (µs) |
|-----------------------|----------|----------|------------|
| submit→issue          | 0.6      | 6.9      | 38.8       |
| issue→complete        | 20.1     | 51.1     | 164.9      |
| complete→wakeup       | [Proxy]  | [Proxy]  | [Proxy]    |
| wakeup→sched          | [Proxy]  | [Proxy]  | [Proxy]    |
| **Residual (Host)**   | **0.03** | **0.09** | **0.38**   |

## Comparison against Total Latency

| Total Latency (p99) | Sum of Segments (p99) | Correlation |
|---------------------|-----------------------|-------------|
| 55.5 µs             | ~58.0 µs              | High        |

## Interpretation

### Where does p99 latency live?

Based on the current data:
1. **Host Submission Path (`submit→issue`)**: Adds ~7µs at p99. This is significant but manageable.
2. **"Device" Jitter (`issue→complete`)**: In this WSL environment, even a busy-wait in userspace is subject to hypervisor de-scheduling, causing p99 to jump from 20µs to 51µs.
3. **Completion Path**: In the baseline (synchronous) mode, completion consumption is extremely fast (~30-90ns) because the `io_uring_enter` call only returns once the work is finished.

### Should we implement CQ polling?

**NO (Preliminary Decision)**.

**Reasoning**:
- The current results show that **Host Submission** and **Hypervisor Jitter** are the dominant factors.
- The residual completion-to-userspace time is currently sub-microsecond in the baseline mode.
- We have not yet proven that the kernel's completion path (`complete→wakeup`) is a bottleneck on native hardware.

## Next Steps
- Move validation to a **Native Linux** host to capture high-fidelity eBPF traces.
- Isolate whether the `issue→complete` jitter is purely a hypervisor artifact or if it contains residual kernel overhead.
