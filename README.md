# SRAM Inference Kernel Fastpath

## TL;DR
In deterministic AI inference (~20µs execution), Linux host overhead can match or exceed device 
latency, effectively doubling end-to-end request time.

In our synthetic SRAM-style workload (~20µs compute), baseline p99 reaches ~40–50µs, indicating 
host overhead comparable to device execution.

This repo isolates that overhead and prototypes the kernel fast paths required to close the gap.

> Once inference becomes deterministic, the Linux control plane—not the model—dominates latency.

This project targets the **post-compute bottleneck regime**, where hardware execution is no 
longer the dominant source of latency.

## What This Repo Demonstrates

- **Deterministic compute does not eliminate latency variance**: Even with zero-variance 
  hardware execution, host-side effects drive significant jitter.
- **Linux submission and completion paths remain significant**: System call overhead and 
  completion delivery pipelines contribute measurable microseconds.
- **Tail latency (p99/p999) is driven by host-side effects**: Scheduling and interrupt 
  handling costs dominate the "tail" of the latency distribution.
- **Existing io_uring fast paths reduce but do not eliminate this gap**: Features like 
  SQPOLL and registered buffers optimize portions of the path but leave completion-side 
  bottlenecks unaddressed.

## Benchmark Tracks

To provide a comprehensive evaluation, the validation harness supports two distinct tracks:
- **NOP mode**: Measures raw `io_uring` ring overhead with minimal operations.
- **SRAM20 mode**: Implements a deterministic AI inference model using a 20µs busy-wait 
  to simulate predictable hardware execution.

## Takeaway

SRAM-style AI inference does not eliminate latency—it exposes the Linux control plane as the 
dominant bottleneck. Closing this gap requires not faster accelerators, but faster kernel paths.

## Important

WSL results are used for **harness validation only**. They are NOT used to draw conclusions about:
- SQPOLL effectiveness
- Kernel scheduling behavior
- Completion latency

All definitive research conclusions require native Linux validation.

## Documentation & Methodology

- [Quickstart Guide](docs/quickstart.md)
- [Native Linux Validation Guide](docs/native-linux-validation.md)
- [Existing io_uring Fast Paths and Remaining Gaps](docs/kernel-patches/existing-io_uring-fastpaths.md)
- [Maintainer FAQ](docs/maintainer-faq.md)
- [Project Roadmap](docs/roadmap.md)

## What This Is / Is Not

**This is:**
- Experimental Linux kernel fast-path research and prototyping.
- Reproducible latency modeling for deterministic AI workloads.
- A measurement-first effort to justify new kernel APIs.

**This is NOT:**
- A production-ready kernel patch (yet).
- A replacement for standard `io_uring` features.
- Performance theater using non-deterministic hardware.

## License
MIT
