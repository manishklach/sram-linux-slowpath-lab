# LKML Positioning Strategy

When submitting these patches to the Linux Kernel Mailing List (LKML), it is critical to frame them correctly to ensure they are seen as general-purpose subsystem improvements rather than niche AI/ML optimizations.

## Core Framing

Do **NOT** use:
- "SRAM chip support"
- "AI special case"
- "LLM optimization"

**Instead, use**:
- "Reducing host-side latency variance for deterministic accelerator workloads"
- "Minimizing GUP overhead in high-frequency IO submission"
- "Optimizing completion wakeup paths for microsecond-scale determinism"

## Key Wording Principles

- **Conservative**: Emphasize that these changes are optional and do not impact default system stability or fairness.
- **Measurable**: Provide raw `bpftrace` or `ftrace` data showing the exact nanosecond improvements.
- **Subsystem-specific**: Talk in terms of `io_uring`, `dma-mapping`, and the scheduler (`sched`).
- **No Hype**: Avoid marketing language; focus on the technical mechanics of the slow path.
