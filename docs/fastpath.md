# Fast Path Runtime Prototype

The `FastPath` runtime is a userspace prototype designed to simulate the latency profile of an "ideal" kernel-optimized inference path.

## What is Fast Path?

Unlike the other simulations which model specific kernel costs (like memory pinning or interrupt delivery), the `FastPath` assumes all such overheads have been removed or bypassed.

- **Persistent Execution Loop**: The application stays in its hot loop, avoiding per-request process management or syscall transitions.
- **No Syscall Overhead**: Models a path where doorbells are written directly from userspace or via a persistent `io_uring` submission queue (SQPOLL).
- **No GUP (Memory Pinning)**: Assumes all buffers are pre-registered and persistently DMA-mapped.
- **Polling Completion**: Uses a tight polling loop for completions, avoiding context switch delays and interrupt overhead.

## What it simulates

This prototype represents the target behavior for our proposed kernel patches:
- **Proposal 1 (Persistent Buffers)**: Removes the submission-time memory mapping cost.
- **Proposal 2 (Polling Mode)**: Removes the completion-time interrupt and wakeup delay.

## Expected Results

In the Fast Path, latency should be dominated by the accelerator compute time:
`total_ns ≈ device_ns + minimal_overhead (~500ns)`

This demonstrates the theoretical floor of latency achievable on the host side without re-architecting the hardware itself.
