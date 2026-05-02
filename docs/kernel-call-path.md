# Conceptual Kernel Call Path

This document describes the end-to-end path of a deterministic inference request through the Linux kernel.

## 1. Submission Path

1. **Userspace**: Host application prepares request.
2. **Syscall**: Call to `io_uring_enter` or standard `write`/`ioctl`.
3. **Kernel Entry**: Entry into the VFS or io_uring subsystem.
4. **Driver Submission**: Driver-specific code prepares descriptors.
5. **Memory Management**:
   - `pin_user_pages`: Pin userspace buffers in memory.
   - `dma_map_sg`: Map buffers for DMA access by hardware.
6. **Device Interaction**: Write to device doorbell/BAR register.

## 2. Completion Path

1. **Device Completion**: Hardware finishes execution.
2. **Hard IRQ**: Hardware signals the host CPU.
3. **IRQ Handler**: Kernel acknowledges the interrupt.
4. **Softirq / Deferred Work**: Kernel processes high-priority cleanup (e.g., `NET_RX_SOFTIRQ`, `BLOCK_SOFTIRQ`).
5. **Wakeup**: Kernel signals that the userspace thread can resume (`sched_wakeup`).
6. **Scheduler**: The kernel scheduler context switches the thread back in (`sched_switch`).
7. **Userspace Resume**: The application receives the completion notification.

## Observation Coverage

The `bpftrace` scripts in `trace/bpf/` observe generic kernel events (`syscalls`, `irq`, `softirq`, `sched`). To observe driver-specific costs like `pin_user_pages` or exact hardware signaling, a native driver and hardware (or a driver stub) would be required.
