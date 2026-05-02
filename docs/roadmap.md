# Project Roadmap

The evolution of the SRAM Linux Slowpath Lab is divided into five distinct phases:

- **Phase 1: WSL Synthetic Experiments** (Complete)
  - Modeled deterministic compute and host overhead.
  - Demonstrated median latency collapse with fixed buffers/polling.

- **Phase 2: Attribution and Visualization** (Complete)
  - Built tools to analyze latency per stage (submit, device, completion).
  - Created visualization layer for latency distribution.

- **Phase 3: Native Linux Tracing** (In Progress)
  - Developing `bpftrace` scripts for native hardware.
  - Capturing real IRQ, softirq, and scheduler wakeup data.

- **Phase 4: Patch Proposal Design** (Current)
  - Formalizing design documents for kernel-level optimizations.
  - Building the case for upstream submission.

- **Phase 5: Prototype Kernel Patches** (Future)
  - Implementing initial patches for persistent buffers and polling.
  - Validating performance improvements on real hardware.
