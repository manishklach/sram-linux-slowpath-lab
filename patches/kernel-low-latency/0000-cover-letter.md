# [PATCH 0/3] io_uring: experimental low-latency completion mode

## Overview
This series introduces a "low-latency completion mode" for `io_uring`, targeting deterministic accelerators (SRAM-based inference) where host-side scheduling variance dominates total roundtrip time.

## Background
Experimental findings indicate that standard asynchronous completion paths incur significant variance due to task_work batching and scheduler wakeup delays. This series provides an opt-in mechanism to prioritize latency via busy-polling and aggressive notification.

## Series Structure
- [PATCH 1/3] io_uring: add experimental low-latency completion mode
- [PATCH 2/3] docs: add io_uring low-latency mode documentation
- [PATCH 3/3] tools: add validation benchmark notes

## Status
RESEARCH PROTOTYPE. Not intended for upstream submission. Local validation only.
No real SRAM hardware validation has been performed yet.
