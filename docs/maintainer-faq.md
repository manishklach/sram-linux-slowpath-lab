# Maintainer FAQ

## Why not just use SQPOLL?
SQPOLL is a powerful mechanism for reducing submission-side syscall overhead by using a kernel thread to poll the SQ. While it addresses one side of the equation, this project focuses on the **completion** side—specifically completion visibility, task wakeup delays, and scheduling latency that SQPOLL does not directly address.

## Why not just use REGISTER_BUFFERS?
Registered buffers are essential for minimizing memory setup and page pinning overhead. They improve the "Submit → Issue" path significantly. However, they do not remove the overhead of completion delivery or the subsequent wakeup and scheduling paths required to notify userspace.

## Why not just use IOPOLL?
IOPOLL is highly effective for supported polling-capable I/O paths (e.g., NVMe). This project first measures whether residual wakeup and scheduler latency remains for deterministic accelerator-style loops that may not fall under standard IOPOLL-compatible categories.

## Why not stop at SQPOLL + registered buffers?
Because even after applying these mechanisms, tail latency may remain due to completion delivery, wakeup, and scheduler behavior. Features like `SQPOLL` optimize the submission plane, but the completion plane—the path from a request being finished to the application becoming active—remains subject to standard kernel wakeup and scheduling logic. This project measures whether that residual gap is significant enough to warrant completion-side fast paths.

## Is this AI-specific?
No. Kernel APIs should remain general-purpose. AI inference is used here as a motivating workload because its deterministic nature (via SRAM-style accelerators) makes host-side latency—which is often buried in "noise"—clearly visible and measurable.

## Are these upstream-ready patches?
No. This repository contains experimental patches and validation tools intended for research and hardware-software co-design discussions. They are not currently proposed for inclusion in the mainline Linux kernel.
