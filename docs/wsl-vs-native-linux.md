# WSL vs Native Linux

This lab is designed to be WSL-compatible, meaning it compiles and runs seamlessly within the Windows Subsystem for Linux (Ubuntu). 

## What can be measured in WSL?
- **Synthetic Latency Modeling**: WSL is excellent for writing synthetic applications that prove out a latency model. We can simulate host overheads deterministically.
- **Userspace Overhead**: General application logic and userspace data transformations.
- **Algorithmic Differences**: Proving that fixed buffers are faster than per-request buffers mathematically/logically.

## What requires Native Linux?
- **Real IRQ / Softirq Tracing**: WSL does not expose real hardware IRQ behavior reliably. Measuring true interrupt latency requires a native kernel.
- **Hardware Page Pinning**: Operations like `pin_user_pages` and real DMA mappings require actual hardware drivers.
- **eBPF Kernel Probes**: While WSL2 supports some eBPF, deep tracing of `sched_switch` or specific driver paths is best done on a bare-metal Linux installation.

The WSL version proves the method and latency model, while the native Linux version provides the real kernel-path proof.
