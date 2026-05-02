# Latency Model

We model the total round-trip time (`total_ns`) of a single request as the sum of three distinct phases:

1. **`submit_ns`**: The time it takes for the host to prepare the request. This includes constructing descriptors, setting up memory (if not pre-registered), and writing to a submission queue.
2. **`device_ns`**: The time the hardware spends actually executing the request. In our SRAM model, this is extremely fast and deterministic (~20μs).
3. **`completion_ns`**: The time from when the device finishes to when the host application actually receives the completion. This includes hardware interrupts, kernel softirqs, and scheduler wakeups.

At SRAM speeds, `submit_ns` and `completion_ns` (the Linux/host slowpath) can easily exceed `device_ns`.
