# Kernel Patch Series: io_uring Latency Attribution

This document describes the first instrumentation patch series for the SRAM/Linux slowpath project.

## Patch Series: [PATCH 0/3] io_uring: add latency attribution tracepoints

### Objective
To bridge the gap between userspace measurements and kernel-side event timing, we require stable tracepoints that capture the transitions between submission, issue, and completion.

### Why Instrumentation First?
Before proposing behavioral changes (such as persistent DMA buffers or scheduler hints), we must have the ability to prove where latency is currently incurred. This instrumentation allows for:
- Computing **Submission Delay** (kernel queue time).
- Computing **Execution Time** (driver/hardware time).
- Correlating kernel events with specific `request_id` values from the userspace benchmark.

### Expected Events
The patches add/update the following tracepoints in the `io_uring` subsystem:
- `io_uring_submit_req`: Triggered when an SQE is accepted.
- `io_uring_issue`: Triggered when the request is issued to the driver.
- `io_uring_complete`: Triggered when the request is finished.

### Latency Equations
Using these tracepoints, latency can be attributed as follows:

```text
Submission Delay = t(io_uring_issue) - t(io_uring_submit_req)
Execution Time   = t(io_uring_complete) - t(io_uring_issue)
Total Overhead   = Submission Delay + (Completion-to-Application time)
```

## Patch Details
The patches are located in `patches/kernel/`:
1. `0001-io_uring-add-submit-and-issue-tracepoints.patch`
2. `0002-io_uring-add-completion-tracepoint.patch`
3. `0003-docs-add-io_uring-latency-attribution-example.patch`

## Positioning
These patches are framed as general-purpose latency attribution tools for any low-latency io_uring workload (Storage, Networking, Accelerators). They do not contain any vendor-specific or AI-specific logic.
