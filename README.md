# SRAM Linux Slowpath Lab

A laboratory for measuring and documenting Linux kernel latency overhead in deterministic SRAM-based inference workloads.

## Core Claim

> **"Once compute is deterministic, the OS becomes the bottleneck."**

Standard Linux submission and completion paths (memory pinning, interrupts, and scheduling) account for over 50% of total latency for 20µs deterministic inference requests.

- [Read the Core Claim](docs/core-claim.md)
- [Final Experimental Results](docs/final-results.md)

## What this repo shows

- **Baseline Linux path doubles latency**: Standard IO patterns add ~20µs of overhead to a 20µs compute task.
- **Fixed buffers reduce submission overhead**: Pre-registering memory removes the cost of per-request GUP/DMA mapping.
- **Polling reduces completion overhead**: Bypassing interrupts eliminates context switch and wakeup jitter.
- **Fastpath demonstrates near-ideal performance**: A fully optimized path collapses latency to near-hardware limits (~20.7µs).

### Comparison Table (p50)

| Mode | p50 Latency | Description |
| :--- | :--- | :--- |
| **Baseline** | ~40.9 µs | Per-request setup + Interrupts |
| **FixedBuf** | ~23.2 µs | Pre-registered buffers |
| **Polling** | ~21.2 µs | Deterministic completion |
| **FastPath** | ~20.7 µs | Ideal optimized path |

## What this is NOT

- **Not real hardware benchmarking**: All hardware execution is simulated via deterministic busy-waits.
- **Not kernel patches yet**: This repo contains the evidence and [proposals](docs/kernel-patch-proposals.md) for future patches.
- **Not claiming upstream results**: This is an experimental framework for systems research.

## Project Roadmap

1. **WSL Synthetic Experiments** (Complete)
2. **Attribution and Visualization** (Complete)
3. **Native Linux Tracing** (Scaffold Ready)
4. **Patch Proposal Design** (Documented)
5. **Prototype Kernel Patches** (Future)

- [Full Roadmap](docs/roadmap.md)
- [Native Linux Tracing Documentation](docs/native-linux-tracing.md)

## Getting Started

```bash
# Build all benchmarks
./scripts/build.sh

# Run the full suite pinned to CPU 0
./scripts/run_all_pinned.sh
```

See `docs/` for deep dives into [attribution](docs/attribution.md) and [kernel paths](docs/kernel-call-path.md).
