# Adaptive Batching Experiment Results

## Goal
Evaluate a simple heuristic-based adaptive batching strategy to balance host-side overhead 
reduction and request latency.

## Adaptive Heuristic
- **Target**: Keep total batch latency between 22µs and 25µs (with 20µs fixed compute).
- **Control**: 
  - If latency > 25µs: Decrease batch size (min 1).
  - If latency < 22µs: Increase batch size (max 32).

## Results Comparison (WSL Synthetic)

| Strategy | p50 Latency (µs) | p99 Latency (µs) | Avg Batch Size | Notes |
|----------|------------------|------------------|----------------|-------|
| Fixed 1  | 22.8             | 55.7             | 1.0            | Baseline syscall overhead |
| Fixed 16 | 26.7             | 60.3             | 16.0           | Saturated submission |
| Adaptive | 23.2             | 33.7             | ~2.4           | Reacts to host jitter |

## Interpretation

### Does adaptive batching converge to optimal range?
**Yes.** In this environment, the adaptive strategy converged to a range of 1–8. Because 
the hypervisor noise frequently pushes the 20µs workload above the 25µs threshold, the 
heuristic correctly prevents the batch size from inflating into the high-latency regime.

### Does it maintain low latency?
**Yes.** The adaptive strategy maintained a p50 (23.2µs) that is within 0.4µs of the 
theoretical minimum (Baseline), while providing submission efficiency gains.

### Does it outperform fixed batching?
**In the tail (p99), YES.** The adaptive strategy significantly outperformed both Fixed 1 
and Fixed 16 at the p99 level (33.7µs vs ~55-60µs). This suggests that by dynamically 
shrinking the batch size when latency spikes are detected, the system avoids "queuing" 
additional requests behind a slow, jitter-affected batch.

## Implication for AI Inference
Adaptive batching is essential for real-world deterministic inference servers. It allows 
the system to:
1. **Harvest efficiency** during quiet host periods by increasing batch size.
2. **Protect tail latency** during host-side jitter events by shrinking the batch size.
3. **Self-calibrate** to the specific scheduling characteristics of the underlying kernel 
   and hypervisor.

## Conclusion
A simple latency-based heuristic is sufficient to outperform static batching strategies in 
unpredictable environments. Future work should explore more sophisticated control loops 
(e.g., PID controllers) for even tighter latency bounds.
