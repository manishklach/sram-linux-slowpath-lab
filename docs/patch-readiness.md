# Patch Readiness Checklist

Before these proposals can be turned into real kernel patches and submitted upstream, they must meet the standards of the Linux kernel community.

## Checklist

- [x] **Initial Proof of Concept**: Synthetic simulation proves the latency impact.
- [x] **Latency Attribution Scaffold**: Measurement tools are ready to capture data.
- [ ] **Native Linux Measurements**: Real data on IRQ delivery and GUP costs from native hardware.
- [ ] **Isolated Benchmark Data**: Evidence that the optimization works independently of simulator noise.
- [ ] **Comparative Baseline**: Measurements against existing mechanisms (e.g., standard `io_uring`).
- [ ] **Maintainer-Friendly Design**: Proposals framed as subsystem improvements rather than "AI special cases".

## Readiness Table

| Proposal | Evidence Today | Missing Evidence | Readiness |
| :--- | :--- | :--- | :--- |
| **Persistent buffers** | Synthetic fixed buffer improvement | Real GUP/dma_map tracing | Medium |
| **Polling completion** | Synthetic polling improvement | Real IRQ/softirq data | Medium |
| **Scheduler hint** | Wakeup-to-sched attribution scaffold | Native Linux measurements | Low-Medium |
| **Tracepoints** | Tracing gap documented | Maintainer-friendly tracepoint design | Low-Medium |
