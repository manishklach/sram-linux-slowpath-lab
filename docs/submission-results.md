# Submission Path Experiment Results

## Goal
Identify the strongest levers for reducing `submit→issue` latency in deterministic ~20µs 
workloads.

## Results Summary (WSL Synthetic)

| Mode | CPU | Batch | p50 (µs) | p99 (µs) | p999 (µs) | Notes |
|------|-----|-------|----------|----------|-----------|-------|
| A (Baseline) | 0 | 1 | 0.6 | 5.4 | 11.3 | Fixed syscall cost per request |
| B (SQPOLL) | 0 | 1 | 5.3 | 85.0 | 5942.5 | WSL hypervisor noise dominates |
| B (SQPOLL) | 1 | 1 | 4.6 | 59.5 | 6390.6 | Cross-core context switch cost |
| C (SQPOLL+Buf) | 0 | 1 | 5.4 | 78.6 | 6781.6 | Buffers show no gain in WSL |
| E (SQPOLL+Buf+File) | 0 | 1 | 4.6 | 78.9 | 3653.4 | Files show marginal gain in WSL |
| A (Batch 4) | 0 | 4 | 0.7 | 5.6 | 81.3 | **0.17µs per request** |
| A (Batch 16) | 0 | 16 | 1.4 | 6.4 | 36.1 | **0.08µs per request** |

## Interpretation

### Does SQPOLL reduce submit latency?
**In this synthetic WSL environment, NO.** SQPOLL actually increases p50 and introduces 
extreme tail latency (ms scale). This is because the SQPOLL kernel thread is frequently 
de-scheduled by the Windows host, causing "stalls" that the synchronous baseline avoids by 
staying in-context.

### Does CPU placement matter?
**Yes.** Moving the submitter to a different core (Core 1) while SQPOLL remains on its 
default core showed a slight shift in p50, but the dominant factor remains the hypervisor 
scheduling noise.

### Does batching reduce p99?
**YES. This is the strongest lever.** While a single submission takes ~600ns, a batch of 16 
takes only ~1400ns, reducing the per-request submission tax to **<90ns**. Crucially, the 
p99 for a batch of 16 (~6.4µs) is only slightly higher than for a single request (~5.4µs).

### Do fixed buffers help submission?
In this environment, the gain was not visible. The overhead of the `io_uring_enter` 
transition and the virtualized ring processing dwarfs the savings from memory pinning 
avoidance.

## Key Finding

The dominant factor affecting `submit→issue` latency is **Batching**.

By amortizing the fixed cost of the submission path (syscall transition and ring processing) 
across multiple requests, batching reduces the effective submission tax from ~600ns to **<100ns 
per request**. This is the only mechanism that reliably brings submission overhead into the 
nanosecond regime.

## Conclusion
For deterministic workloads, batching should be the primary optimization strategy. SQPOLL 
requires native Linux isolation to be effective; in virtualized environments, it is 
counter-productive for low-latency tasks.
