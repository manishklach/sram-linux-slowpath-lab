# Core Claim: The Shift to Control Plane Bottlenecks

"SRAM-based inference shifts the bottleneck from compute to the Linux control plane. This repo prototypes the missing kernel fast paths required to close that gap."

## Rationale

In standard AI workloads, compute (Matrix Multiplication, Attention) is highly variable and dominated by memory bandwidth jitter and non-deterministic cache hierarchies. In this environment, the 20-50µs overhead of the Linux kernel is "noise."

However, next-generation **SRAM-based accelerators** provide near-deterministic compute times. When a hardware device can complete an inference request in exactly 20µs, the Linux kernel's submission and completion path overhead (~20-40µs) becomes the dominant factor in end-to-end latency.

## The Gap

The Linux kernel lacks a specialized "Inference Fast Path" that:
1. Eliminates per-request memory management (GUP/DMA) overhead.
2. Bypasses the interrupt/wakeup slowpath via bounded polling.
3. Provides first-class attribution for hardware vs. software delay.

This project exists to close that gap.
