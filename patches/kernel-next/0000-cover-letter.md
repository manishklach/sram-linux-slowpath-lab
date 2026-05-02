# [PATCH 0/4] io_uring: registered-buffer enforcement and wakeup attribution

## Overview
This patch series continues the work on SRAM-style AI inference slowpath analysis. It focuses on two key areas:
1. **Registered-Buffer Fast Path (Patch Set C)**: Improving visibility and enforcement of fixed buffers in low-latency rings.
2. **Wakeup Attribution (Patch Set E)**: Adding tracepoints to measure the delay between kernel completion and userspace task execution.

## Motivation
Our experiments showed that:
- Per-request memory pinning adds ~20µs of overhead.
- Scheduling delays (wakeup -> execution) can be significant even for "completed" IO.

## Patches
- 0001: Adds `fixed_buffers` field to core tracepoints.
- 0002: Adds a kernel warning when low-latency rings use non-fixed buffers.
- 0003: Adds `io_uring_cq_wakeup` tracepoint.
- 0004: Provides documentation and tracing examples.

## Status
Experimental / Research Prototype. Do not submit upstream.
