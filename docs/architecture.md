# Architecture

The lab is composed of several micro-benchmarks that simulate the end-to-end latency of a request to an ultra-fast hardware accelerator (modeled as SRAM).

## Components

1. **Host Application**: Submits requests and waits for completions.
2. **Device Simulation**: A highly deterministic busy-wait loop that simulates 20μs of SRAM execution time.
3. **Slowpath Overhead Simulation**:
   - **Memory Setup**: Simulating the cost of pinning memory and mapping it for DMA (`pin_user_pages`, `dma_map_sg`).
   - **Completion Overhead**: Simulating the cost of interrupts, softirqs, and context switches vs. polling.

## Lab Progressions

1. **Baseline**: Includes per-request memory setup and standard completion overhead.
2. **Fixed Buffers**: Simulates pre-registered memory buffers, removing setup time from the hot loop.
3. **Polling vs. Interrupts**: Contrasts the variable delay of an interrupt-driven completion with the deterministic (but CPU-intensive) polling approach.
