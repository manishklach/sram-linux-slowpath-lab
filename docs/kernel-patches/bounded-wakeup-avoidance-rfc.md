# Bounded Completion Wakeup-Avoidance RFC

## Motivation
Even when advanced `io_uring` features like `SQPOLL`, `IOPOLL`, registered buffers, and fixed files are utilized, deterministic AI inference (targeting ~20µs execution) still faces microsecond-scale tail latency. Analysis suggests that a significant portion of this residual latency is spent in the transition from kernel completion to userspace task execution:
`completion posted → wakeup requested → task scheduled`.

## Question
Can an opt-in, bounded completion-side wait avoid unnecessary task wakeups when the userspace application is already actively consuming the Completion Queue (CQ)?

## Why this is not SQPOLL
`SQPOLL` is designed to optimize the **submission** path by polling the Submission Queue (SQ) from a dedicated kernel thread. This RFC addresses the **completion** side—specifically the overhead of the wakeup and scheduling path for the user process once the compute is finished.

## Why this is not IOPOLL
`IOPOLL` is used to poll for device completions on supported I/O paths (e.g., block devices). This RFC is broader: it explores whether a posted CQE (of any type) can be consumed by userspace during a short kernel-side "grace period," thereby avoiding the expensive `__wake_up` call and subsequent scheduler overhead entirely.

## Proposed experiment
- **No new ABI initially**: This is an internal research exploration.
- **Controlled Prototyping**: Any behavioral changes should be gated behind a debug/config flag or maintained on a research-only branch.
- **Trace-First Approach**: Before any policy change is evaluated, we must trace `complete → wakeup` and `wakeup → sched-in` to quantify the actual overhead on native hardware.
- **Evidence-Based Pivot**: Behavior changes should only be considered if trace data confirms that the wakeup/scheduling path is a dominant factor in tail latency.

## Safety constraints
- **Bounded Wait Only**: The kernel-side spin must be strictly time-bounded (e.g., <20µs).
- **Yielding**: The loop must break immediately on `need_resched()`.
- **Signal Awareness**: The loop must break on `signal_pending()`.
- **Reliable Fallback**: If the "grace period" expires without CQ consumption, the kernel must fall back to the existing, standard wakeup path.
- **Non-Destructive**: No modification to existing Linux scheduling or fairness logic.
- **Zero Default Change**: No change to standard Linux behavior for non-opt-in workloads.

## Acceptance criteria
Advancing from design to implementation is only justified if:
1. **Empirical Evidence**: Native Linux traces show that `completion → wakeup` or `wakeup → sched-in` contributes significantly to p99 latency spikes.
2. **Exhaustion of Alternatives**: Existing `SQPOLL`, `IOPOLL`, and `registered-buffer` modes are confirmed to be insufficient for resolving these specific tail latency gaps.
3. **High Hit Potential**: Preliminary data suggests a favorable hit/fallback ratio, indicating that userspace is often active enough to consume the CQ within the grace period.
