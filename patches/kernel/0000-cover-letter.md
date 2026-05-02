# [PATCH 0/3] io_uring: add latency attribution tracepoints

## Context
For deterministic accelerators (SRAM-based inference, low-latency storage), the Linux control plane overhead often exceeds the actual device execution time. Diagnosing exactly where this latency is incurred requires high-fidelity instrumentation of the request lifecycle within `io_uring`.

This patch series adds and refines tracepoints to enable request-level latency attribution.

## Goals
- Enable microsecond-accurate measurement of submission-to-issue and issue-to-completion delays.
- Capture enough context (pid, tgid, user_data) to align kernel events with userspace application logs.
- Provide documentation and examples for systems researchers.

## Series Overview

### [PATCH 1/3] io_uring: add submit and issue tracepoints
- Adds `pid` and `tgid` to `io_uring_submit_req`.
- Introduces `io_uring_issue` tracepoint called at the point of dispatch.

### [PATCH 2/3] io_uring: add completion tracepoint refinement
- Adds `pid` and `tgid` to `io_uring_complete`.
- Enables full-path attribution from a single task context.

### [PATCH 3/3] docs: add io_uring latency attribution example
- Documents the attribution equations and provides a `bpftrace` example.

## Framing
This is an instrumentation-only series. It does not modify any scheduling, memory management, or execution logic. It is intended for researchers and developers optimizing deterministic workloads.
