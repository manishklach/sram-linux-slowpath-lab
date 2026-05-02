# Latency Model

We model the total round-trip time (`total_ns`) of a single request as the sum of three distinct phases:

1. **`submit_ns`**: The time it takes for the host to prepare the request. This includes constructing descriptors, setting up memory (if not pre-registered), and writing to a submission queue.
2. **`device_ns`**: The time the hardware spends actually executing the request. In our SRAM model, this is extremely fast and deterministic (~20μs).
3. **`completion_ns`**: The time from when the device finishes to when the host application actually receives the completion. This includes hardware interrupts, kernel softirqs, and scheduler wakeups.

At SRAM speeds, `submit_ns` and `completion_ns` (the Linux/host slowpath) can easily exceed `device_ns`.

## Tail at Scale in Inference

Modern inference pipelines suffer from **Tail Amplification**:
- An end-to-end request consists of multiple sequential stages (submission, memory pinning, compute, interrupt, scheduling).
- Even if each stage has a small probability of jitter, the combined probability of hitting a tail event at any one stage is much higher.
- **Deterministic compute (SRAM)** removes one source of jitter, but it exposes the host OS overhead as the now-dominant source of variance.
- At p999, the latency is almost entirely defined by OS scheduling noise rather than the workload itself.
