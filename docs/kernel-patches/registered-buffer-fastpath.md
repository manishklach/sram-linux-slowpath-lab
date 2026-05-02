# Kernel Patch: Registered-Buffer Fast Path

## Problem
In standard `io_uring` read/write operations, the kernel must perform `pin_user_pages` and `dma_map_sg` for every single request. In our [Fixed Buffer Simulation](fixed_buffers_sim.c), we found that this adds ~17-20µs of overhead.

For a 20µs deterministic inference request, this effectively doubles the latency.

## Solution
Registered buffers (`IORING_REGISTER_BUFFERS`) allow the application to pin and map memory once at startup. Subsequent IO operations just pass an index into the registered table.

This patch adds:
1. **Visibility**: A `fixed_buffers` boolean to `io_uring` tracepoints.
2. **Policy Hint**: A kernel warning when `IORING_SETUP_LOW_LATENCY` is used without fixed buffers for standard IO ops.

## Connection to Results
By enforcing/recommending fixed buffers, we ensure that the "Low Latency" mode isn't undermined by the expensive memory management slowpath.
